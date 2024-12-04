#include "BlinkyPicoW.h"

BlinkyPicoW::BlinkyPicoW(boolean init,  PubSubClient* mqttClient, volatile boolean* pcubeHasDataToRead, volatile boolean* pmqttHasDataToRead, volatile boolean* pforceArchiveData)
{
  m_init = init;
  m_resetButtonValue = LOW;
  m_resetButtonSwitchTime = 0;
  m_wifiAccessPointMode = false;
  m_webPageServed = false;
  m_webPageRead = false;
  m_mqttClient = mqttClient;
  m_mqttDataHeader.state = 0;
  m_mqttDataHeader.forceArchiveData = 0;
  m_mqttDataHeader.watchdog = 0;
  for (int ii = 0; ii < 6; ++ii)
  {
    m_mqttDataHeader.deviceMac[ii] = 0;
    m_mqttDataHeader.routerMac[ii] = 0;
  }
  m_pcubeHasDataToRead = pcubeHasDataToRead;
  m_pmqttHasDataToRead = pmqttHasDataToRead;
  m_pforceArchiveData  = pforceArchiveData;
  m_sizeofMqttDataHeader = sizeof(m_mqttDataHeader);
  
}
void BlinkyPicoW::begin(int chattyCathy, int commLEDPin, int resetButtonPin, boolean useFlashStorage, size_t cubeSetting, size_t cubeReading)
{
  rp2040.wdt_begin(m_hdwrWatchdogMs);

  m_sizeofCubeSetting = cubeSetting;
  m_sizeofCubeReading = cubeReading;
  size_t sizeOfTransferData = m_sizeofCubeSetting + m_sizeofCubeReading + m_sizeofMqttDataHeader;
  m_pcubeDataSend = new (std::nothrow) uint8_t [sizeOfTransferData];
  m_pcubeDataRecv = new (std::nothrow) uint8_t [sizeOfTransferData];

  *m_pcubeHasDataToRead = false;
  *m_pmqttHasDataToRead = false;

  m_chattyCathy = false;
  if (chattyCathy > 0) m_chattyCathy = true;
  m_commLEDPin = commLEDPin;
  pinMode(m_commLEDPin, OUTPUT);
  setCommLEDPin(false);
  m_resetButtonPin = resetButtonPin;
  if (m_resetButtonPin > 0 )
  { 
    pinMode(m_resetButtonPin, INPUT);
  }
  m_useFlashStorage = useFlashStorage;
  int idot = 0;
  while (idot < STARTUP_DELAY)
  {
    delay(50);
    rp2040.wdt_reset();;
    idot = idot + 50;
    setCommLEDPin(!m_commLEDState);
  }
  setCommLEDPin(false);
  if (m_chattyCathy)
  {
    Serial.println("");
    Serial.println("Beginning BlinkyPicoWCube");
    Serial.print("Size of transfer data: ");
    Serial.print(sizeOfTransferData);
    Serial.println(" bytes");
  }
  if ((m_pcubeDataSend == nullptr) || (m_pcubeDataRecv == nullptr))
  {
    if (m_chattyCathy) Serial.println("MQTT Memory not allocated!!!");
  }
  if (m_useFlashStorage)
  {
    if (m_chattyCathy)
    {
      Serial.println("Reading creds.txt file");
      Serial.println("    Data:");
    }
    LittleFS.begin();
    File file = LittleFS.open("/creds.txt", "r");
    if (file) 
    {
        String lines[9];
        String data = file.readString();
        int startPos = 0;
        int stopPos = 0;
        for (int ii = 0; ii < 9; ++ii)
        {
          startPos = data.indexOf("{") + 1;
          stopPos = data.indexOf("}");
          lines[ii] = data.substring(startPos,stopPos);
          if (m_chattyCathy)
          {
            Serial.print("        ");
            Serial.println(lines[ii]);
          }
          data = data.substring(stopPos + 1);
        }
        m_ssid               = lines[0];
        m_wifiPassword       = lines[1];
        m_mqttServer         = lines[2];
        m_mqttUsername       = lines[3];
        m_mqttPassword       = lines[4];
        m_box                = lines[5];
        m_trayType           = lines[6];
        m_trayName           = lines[7];
        m_cubeType           = lines[8];
        file.close();
    }
    else
    {
      if (m_chattyCathy) Serial.println("file open failed");
    }
  }
  m_mqttSubscribeTopic = m_box + "/" + m_cubeType + "/" + m_trayType + "/" + m_trayName + "/setting";
  m_mqttPublishTopic   = m_box + "/" + m_cubeType + "/" + m_trayType + "/" + m_trayName + "/reading";
  if (m_chattyCathy)
  {
    Serial.println("    Variables:");
    Serial.print("        m_ssid:             ");
    Serial.println(m_ssid);
    Serial.print("        wifiPassword:       ");
    Serial.println(m_wifiPassword);
    Serial.print("        mqttServer:         ");
    Serial.println(m_mqttServer);
    Serial.print("        mqttUsername:       ");
    Serial.println(m_mqttUsername);
    Serial.print("        mqttPassword:       ");
    Serial.println(m_mqttPassword);
    Serial.print("        trayType:           ");
    Serial.println(m_trayType);
    Serial.print("        trayName:           ");
    Serial.println(m_trayName);
    Serial.print("        cubeType            ");
    Serial.println(m_cubeType);
    Serial.print("        mqttPublishTopic:   ");
    Serial.println(m_mqttPublishTopic);
    Serial.print("        mqttSubscribeTopic: ");
    Serial.println(m_mqttSubscribeTopic);
    Serial.println("");
  }
  m_mqttClient->setServer(m_mqttServer.c_str(),m_mqttPort);
  m_mqttClient->setCallback (mqttCallback);
  m_mqttClient->setKeepAlive(m_mqttKeepAlive);
  m_mqttClient->setSocketTimeout(m_mqttSocketTimeout);

  setup_wifi();
  
  return;
}

void BlinkyPicoW::loop()
{
  rp2040.wdt_reset();;
  unsigned long nowTime = millis();
  if (m_wifiAccessPointMode)
  {
    serveWebPage();
    readWebPage();
    rp2040.wdt_reset();;
    delay(100);
    return;
  }
// Check reset button
  if (m_resetButtonPin > 0 )
  {
    int pinValue = digitalRead(m_resetButtonPin);
    if (pinValue != m_resetButtonValue)
    {
      if((nowTime - m_resetButtonSwitchTime) > DEBOUNCE_TIME)
      {
        m_resetButtonValue = pinValue;
        m_resetButtonSwitchTime = nowTime;
        if (m_chattyCathy)
        {
          if (m_resetButtonValue == HIGH)
          {
            Serial.print("Reset Button Pressed. ");
          }
          else
          {
            Serial.println("Reset Button Released");
          }
        }
      }
    }
    if (m_resetButtonValue == HIGH)
    {
      if((nowTime - m_resetButtonSwitchTime) > RESET_TIMEOUT)
      {
        if (m_chattyCathy)
        {
          Serial.print("Reset Button held down for ");
          Serial.print(RESET_TIMEOUT);
          Serial.println(" mS");
        }
        resetButtonPressed();
        return;
      }
    }
  }
// check wifi
  m_wifiStatus =  WiFi.status();
  if (m_wifiStatus != WL_CONNECTED)
  {
    if (m_chattyCathy) Serial.println("Wifi disconnected. Restarting Wifi.");
    return;
  }
  checkMqttConnection();
  m_mqttClient->loop();
  if (*m_pcubeHasDataToRead)
  {
    if (m_chattyCathy) Serial.println("Publishing MQTT data....");
    m_mqttDataHeader.forceArchiveData = 0;
    if(*m_pforceArchiveData) m_mqttDataHeader.forceArchiveData = 1;
    m_mqttDataHeader.watchdog = m_mqttDataHeader.watchdog + 1;
    if (m_mqttDataHeader.watchdog > 65534) m_mqttDataHeader.watchdog = 0;
    
    uint8_t* memPtr = m_pcubeDataSend;
    uint8_t* datPtr = (uint8_t*) &m_mqttDataHeader;
    for (int ii = 0; ii < m_sizeofMqttDataHeader; ++ii)
    {
      *memPtr = *datPtr;
      ++memPtr;
      ++datPtr;
    }
    int numBytes = m_sizeofCubeSetting + m_sizeofCubeReading +  m_sizeofMqttDataHeader;
    if (m_chattyCathy) 
    {
      uint8_t* ptr = m_pcubeDataSend;
      for (int ii = 0; ii < numBytes; ++ii)
      {
        Serial.print("    ptr: ");
        Serial.print((uint32_t) ptr);
        Serial.print(" byte: ");
        Serial.println((uint8_t) *ptr);
        ++ptr;
      }
    }
    if(m_mqttClient->publish(m_mqttPublishTopic.c_str(), m_pcubeDataSend, numBytes))
    {
      *m_pcubeHasDataToRead = false;
      *m_pforceArchiveData = false;
      if (m_chattyCathy) Serial.println("successful!");
      setCommLEDPin(true);
      m_lastMqttPublishTime = nowTime;


    }
    else
    {
      if (m_chattyCathy) Serial.println("failed.");
    }
 
  }
  if (m_commLEDState)
  {
    if ((nowTime - m_lastMqttPublishTime) > m_mqttLedFlashMs) 
    {
      setCommLEDPin(false);
    }
  }
  delay(1);
  return;
}
void BlinkyPicoW::setup_wifi() 
{
  if (m_chattyCathy) Serial.println();
  if (m_chattyCathy) Serial.println("Starting Communications");
  setCommLEDPin(true);
  WiFi.mode(WIFI_STA); //This will non-block the wifi begin
  int itry = 0;
  while (itry < MAX_NO_CONNECTION_ATTEMPTS)
  {
    itry = itry + 1;
    if (m_chattyCathy)
    {
      Serial.print("    Attempt: ");
      Serial.print(itry);
      Serial.print("/");
      Serial.print(MAX_NO_CONNECTION_ATTEMPTS);
      Serial.print(". Connecting to ");
      Serial.print(m_ssid);
      Serial.print(" with password: ");
      Serial.println(m_wifiPassword);
    }
  
    if (m_wifiPassword.equals("NONE"))
    {
      if (m_chattyCathy ) Serial.println("    Connecting with no password");
      WiFi.begin(m_ssid.c_str());
    }
    else
    {
      WiFi.begin(m_ssid.c_str(), m_wifiPassword.c_str());
    }
    m_wifiStatus = WL_CONNECTED + 1;
    rp2040.wdt_reset();;
    delay(1000);
    setCommLEDPin(false);
    if (m_chattyCathy) 
    {
      Serial.print("    Waiting ");
      Serial.print(WIFI_WAIT);
      Serial.print(" mS for wifi.");
    }
    int iwait = 0;
    while (iwait < WIFI_WAIT)
    {
      rp2040.wdt_reset();;
      delay(500);
      m_wifiStatus =  WiFi.status();
      if (m_wifiStatus == WL_CONNECTED) iwait = WIFI_WAIT;
      iwait = iwait + 500;
      if (m_chattyCathy) Serial.print(".");
      setCommLEDPin(!m_commLEDState);
      if (m_resetButtonPin > 0 )
      {
        if (digitalRead(m_resetButtonPin) == HIGH)
        {
          if (m_chattyCathy)
          {
            Serial.println("");
            Serial.println("Reset Button Pressed. ");
          }
          resetButtonPressed();
          return;
        }
      }
    }
    if (m_chattyCathy) Serial.println("");
    setCommLEDPin(false);
    m_wifiStatus =  WiFi.status();
    if (m_chattyCathy )
    {
      if (m_wifiStatus != WL_CONNECTED)
      {
        Serial.println("    Wifi not connected.");
        Serial.println("");
      }
      else
      {
        itry = MAX_NO_CONNECTION_ATTEMPTS + 1;
      }
    }
  }
  
  if (m_chattyCathy) Serial.println("");
  setCommLEDPin(false);
  if ( m_wifiStatus == WL_CONNECTED)
  {
    if (m_chattyCathy)
    {
      Serial.print("    WiFi connected with IP address: ");
      Serial.println(WiFi.localIP());
      WiFi.BSSID(m_mqttDataHeader.routerMac);
      Serial.print("    Router MAC: ");
      Serial.print(m_mqttDataHeader.routerMac[0],HEX);
      Serial.print(":");
      Serial.print(m_mqttDataHeader.routerMac[1],HEX);
      Serial.print(":");
      Serial.print(m_mqttDataHeader.routerMac[2],HEX);
      Serial.print(":");
      Serial.print(m_mqttDataHeader.routerMac[3],HEX);
      Serial.print(":");
      Serial.print(m_mqttDataHeader.routerMac[4],HEX);
      Serial.print(":");
      Serial.println(m_mqttDataHeader.routerMac[5],HEX);

      WiFi.macAddress(m_mqttDataHeader.deviceMac);
      Serial.print("    Device MAC: ");
      Serial.print(m_mqttDataHeader.deviceMac[0],HEX);
      Serial.print(":");
      Serial.print(m_mqttDataHeader.deviceMac[1],HEX);
      Serial.print(":");
      Serial.print(m_mqttDataHeader.deviceMac[2],HEX);
      Serial.print(":");
      Serial.print(m_mqttDataHeader.deviceMac[3],HEX);
      Serial.print(":");
      Serial.print(m_mqttDataHeader.deviceMac[4],HEX);
      Serial.print(":");
      Serial.println(m_mqttDataHeader.deviceMac[5],HEX);
      Serial.println("");
    }
    m_init = false;
    checkMqttConnection();    

  }
  else
  {
    if (m_chattyCathy)  Serial.println("Too many wifi attempts. Rebooting...");
    delay(1000);
    rp2040.reboot();
  }

}
void BlinkyPicoW::resetButtonPressed()
{
  if (m_wifiAccessPointMode) return;
  if (!m_useFlashStorage)
  {
    if (m_chattyCathy)  Serial.println("Rebooting...");
    delay(1000);
    rp2040.reboot();
  }
  setCommLEDPin(true);
  WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  IPAddress local_IP(192,168,42,1);
  IPAddress gateway(192,168,42,1);
  IPAddress subnet(255,255,255,0);
  WiFi.softAPConfig(local_IP, gateway, subnet);

  WiFi.softAP(m_trayType + "-" + m_trayName);
  if (m_chattyCathy)
  {
    Serial.print("Access Point Created with IP Gateway ");
    Serial.println(WiFi.softAPIP());
  }
  m_wifiServer = new WiFiServer(80);
  m_wifiAccessPointMode = true;
  m_wifiServer->begin();
  m_webPageServed = false;
  m_webPageRead = false;
}
void BlinkyPicoW::readWebPage()
{
  if (!m_webPageServed) return;
  if (m_webPageRead) return;
  WiFiClient client = m_wifiServer->accept();

  if (client) 
  {
    String header = "";
    while (client.connected() && !m_webPageRead) 
    {
      rp2040.wdt_reset();;
      if (client.available()) 
      {
        char c = client.read();
        header.concat(c);
      }
      else
      {
        break;
      }
    }
    if (header.indexOf("POST") >= 0)
    {
      String data = header.substring(header.indexOf("ssid="),header.length());
      m_ssid               = replaceHtmlEscapeChar(data.substring(5,data.indexOf("&pass=")));
      m_wifiPassword       = replaceHtmlEscapeChar(data.substring(data.indexOf("&pass=")  + 6,data.indexOf("&serv=")));
      m_mqttServer         = replaceHtmlEscapeChar(data.substring(data.indexOf("&serv=")  + 6,data.indexOf("&unam=")));
      m_mqttUsername       = replaceHtmlEscapeChar(data.substring(data.indexOf("&unam=")  + 6,data.indexOf("&mpas=")));
      m_mqttPassword       = replaceHtmlEscapeChar(data.substring(data.indexOf("&mpas=")  + 6,data.indexOf("&bbox=")));
      m_box                = replaceHtmlEscapeChar(data.substring(data.indexOf("&bbox=")  + 6,data.indexOf("&tryt=")));
      m_trayType           = replaceHtmlEscapeChar(data.substring(data.indexOf("&tryt=")  + 6,data.indexOf("&tryn=")));
      m_trayName           = replaceHtmlEscapeChar(data.substring(data.indexOf("&tryn=")  + 6,data.indexOf("&cube=")));
      m_cubeType           = replaceHtmlEscapeChar(data.substring(data.indexOf("&cube=")  + 6,data.length()));
      m_mqttSubscribeTopic = m_box + "/" + m_cubeType + "/" + m_trayType + "/" + m_trayName + "/setting";
      m_mqttPublishTopic   = m_box + "/" + m_cubeType + "/" + m_trayType + "/" + m_trayName + "/reading";
      if (m_chattyCathy)
      {
        Serial.println("    Reading returned web form.");
        Serial.println("");
        Serial.print("    m_ssid:             ");
        Serial.println(m_ssid);
        Serial.print("    wifiPassword:       ");
        Serial.println(m_wifiPassword);
        Serial.print("    mqttServer:         ");
        Serial.println(m_mqttServer);
        Serial.print("    mqttUsername:       ");
        Serial.println(m_mqttUsername);
        Serial.print("    mqttPassword:       ");
        Serial.println(m_mqttPassword);
        Serial.print("    trayType:           ");
        Serial.println(m_trayType);
        Serial.print("    trayName:           ");
        Serial.println(m_trayName);
        Serial.print("    cubeType            ");
        Serial.println(m_cubeType);
        Serial.print("    mqttPublishTopic:   ");
        Serial.println(m_mqttPublishTopic);
        Serial.print("    mqttSubscribeTopic: ");
        Serial.println(m_mqttSubscribeTopic);
        Serial.println("");
      }
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html");
      client.println("Connection: close");  // the connection will be closed after completion of the response
      client.println();
      client.println("<!DOCTYPE HTML>");
      client.println("<html><head><title>Blinky-Lite Credentials</title><style>");
      client.println("body{background-color: #083357 !important;font-family: Arial;}");
      client.println(".labeltext{color: white;font-size:250%;}");
      client.println(".formtext{color: black;font-size:250%;}");
      client.println(".cell{padding-bottom:25px;}");
      client.println("</style></head><body>");
      client.println("<h1 style=\"color:white;font-size:300%;text-align: center;\">Blinky-Lite Credentials</h1><hr><div>");
      client.println("<div>");
      client.println("<h1 style=\"color:yellow;font-size:300%;text-align: center;\">Accepted</h1><div>");
      client.println("</div>");
      client.println("</body></html>");
      m_webPageRead = true;
    }
    rp2040.wdt_reset();;
    delay(100);
    client.stop();
    if (m_webPageRead)
    {
      m_wifiAccessPointMode = false;
      WiFi.disconnect();
      if (m_chattyCathy) Serial.println("    Writing creds.txt file");
      File file = LittleFS.open("/creds.txt", "w");
      if (file) 
      {
        String line = "";
        line = "{" + m_ssid + "}";
        file.println(line);
        line = "{" + m_wifiPassword + "}";
        file.println(line);
        line = "{" + m_mqttServer + "}";
        file.println(line);
        line = "{" + m_mqttUsername + "}";
        file.println(line);
        line = "{" + m_mqttPassword + "}";
        file.println(line);
        line = "{" + m_box + "}";
        file.println(line);
        line = "{" + m_trayType + "}";
        file.println(line);
        line = "{" + m_trayName + "}";
        file.println(line);
        line = "{" + m_cubeType + "}";
        file.println(line);
        file.println("");
        file.println("");
        file.println("");
        file.close();
      }
      else
      {
        if (m_chattyCathy) Serial.println("    File open failed");
      }
      if (m_chattyCathy) 
      {
        Serial.println("    File writing complete.");
        Serial.println("    Rebooting");
      }
      rp2040.reboot();

    }
  }
}
void BlinkyPicoW::serveWebPage()
{
  if (m_webPageServed) return;
  WiFiClient client = m_wifiServer->accept();

  if (client) 
  {
    if (m_chattyCathy)
    {
      Serial.println("    new client. Serving webpage.");
      Serial.println("");
    }
    String header = "";
    while (client.connected() && !m_webPageServed) 
    {
      rp2040.wdt_reset();;
      if (client.available()) 
      {
        char c = client.read();
        header.concat(c);
      }
      else
      {
        break;
      }
    }
//    if (m_chattyCathy) Serial.print("    Returned header: ");
//    if (m_chattyCathy) Serial.println(header);
    if (header.indexOf("Upgrade-Insecure-Requests: 1") < 0) return;
    
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");  // the connection will be closed after completion of the response
    client.println();
    client.println("<!DOCTYPE HTML>");
    client.println("<html><head><title>Blinky-Lite Credentials</title><style>");
    client.println("body{background-color: #083357 !important;font-family: Arial;}");
    client.println(".labeltext{color: white;font-size:250%;}");
    client.println(".formtext{color: black;font-size:250%;}");
    client.println(".cell{padding-bottom:25px;}");
    client.println("</style></head><body>");
    client.println("<h1 style=\"color:white;font-size:300%;text-align: center;\">Blinky-Lite Credentials</h1><hr><div>");
    client.println("<form action=\"/disconnected\" method=\"POST\">");
    client.println("<table align=\"center\">");
    
    client.println("<tr>");
    client.println("<td class=\"cell\"><label for=\"ssid\" class=\"labeltext\">SSID</label></td>");
    String tag = "<td class=\"cell\"><input name=\"ssid\" id=\"ssid\" type=\"text\" value=\"" + m_ssid + "\" class=\"formtext\"/></td>";
    client.println(tag.c_str());
    client.println("</tr>");
    
    client.println("<tr>");
    client.println("<td class=\"cell\"><label for=\"pass\" class=\"labeltext\">Wifi Password</label></td>");
    tag = "<td class=\"cell\"><input name=\"pass\" id=\"pass\" type=\"password\" value=\"" + m_wifiPassword + "\" class=\"formtext\"/></td>";
    client.println(tag.c_str());
    client.println("</tr>");
    
    client.println("<tr>");
    client.println("<td class=\"cell\"><label for=\"serv\" class=\"labeltext\">MQTT Server</label></td>");
    tag = "<td class=\"cell\"><input name=\"serv\" id=\"serv\" type=\"text\" value=\"" + m_mqttServer + "\" class=\"formtext\"/></td>";
    client.println(tag.c_str());
    client.println("</tr>");
    
    client.println("<tr>");
    client.println("<td class=\"cell\"><label for=\"unam\" class=\"labeltext\">MQTT Username</label></td>");
    tag = "<td class=\"cell\"><input name=\"unam\" id=\"unam\" type=\"text\" value=\"" + m_mqttUsername + "\" class=\"formtext\"/></td>";
    client.println(tag.c_str());
    client.println("</tr>");
    
    client.println("<tr>");
    client.println("<td class=\"cell\"><label for=\"mpas\" class=\"labeltext\">MQTT Password</label></td>");
    tag = "<td class=\"cell\"><input name=\"mpas\" id=\"mpas\" type=\"password\" value=\"" + m_mqttPassword + "\" class=\"formtext\"/></td>";
    client.println(tag.c_str());
    client.println("</tr>");
   
    client.println("<tr>");
    client.println("<td class=\"cell\"><label for=\"bbox\" class=\"labeltext\">Box</label></td>");
    tag = "<td class=\"cell\"><input name=\"bbox\" id=\"bbox\" type=\"text\" value=\"" + m_box + "\" class=\"formtext\"/></td>";
    client.println(tag.c_str());
    client.println("</tr>");

    client.println("<tr>");
    client.println("<td class=\"cell\"><label for=\"tryt\" class=\"labeltext\">Tray Type</label></td>");
    tag = "<td class=\"cell\"><input name=\"tryt\" id=\"tryt\" type=\"text\" value=\"" + m_trayType + "\" class=\"formtext\"/></td>";
    client.println(tag.c_str());
    client.println("</tr>");
      
    client.println("<tr>");
    client.println("<td class=\"cell\"><label for=\"tryn\" class=\"labeltext\">Tray Name</label></td>");
    tag = "<td class=\"cell\"><input name=\"tryn\" id=\"tryn\" type=\"text\" value=\"" + m_trayName + "\" class=\"formtext\"/></td>";
    client.println(tag.c_str());
    client.println("</tr>");
      
    client.println("<tr>");
    client.println("<td class=\"cell\"><label for=\"cube\" class=\"labeltext\">Cube Type</label></td>");
    tag = "<td class=\"cell\"><input name=\"cube\" id=\"cube\" type=\"text\" value=\"" + m_cubeType + "\" class=\"formtext\"/></td>";
    client.println(tag.c_str());
    client.println("</tr>");
    
    client.println("<tr>");
    client.println("<td></td>");
    client.println("<td><input type=\"submit\" class=\"formtext\"/></td>");
    client.println("</tr>");
    client.println("</table>");
    client.println("</form>");
    client.println("</div>");
    client.println("</body></html>");
    m_webPageRead = false;
    m_webPageServed = true;
    rp2040.wdt_reset();;
    delay(100);
    client.stop();
  }
}
String BlinkyPicoW::replaceHtmlEscapeChar(String inString)
{
  String outString = "";
  char hexBuff[3];
  char *hexptr;
  for (int ii = 0; ii < inString.length(); ++ii)
  {
    char testChar = inString.charAt(ii);
    if (testChar == '%')
    {
      inString.substring(ii + 1,ii + 3).toCharArray(hexBuff, 3);
      char specialChar = (char) strtoul(hexBuff, &hexptr, 16);
      outString += specialChar;
      ii = ii + 2;
    }
    else
    {
      outString += testChar;
    }
  }
  return outString;
}

void BlinkyPicoW::checkMqttConnection() 
{
  boolean connected = m_mqttClient->connected();
  if (connected) return;
  if (m_chattyCathy) 
  {
    Serial.println();
    Serial.println("Connecting to MQTT Broker");
  }
  int itry = 0;
  String mqttClientId = m_trayType + "-" + m_trayName;
  while (itry < MAX_NO_CONNECTION_ATTEMPTS)
  {
    itry = itry + 1;
    if (m_chattyCathy) 
    {
      Serial.print("    Attempt: ");
      Serial.print(itry);
      Serial.print("/");
      Serial.print(MAX_NO_CONNECTION_ATTEMPTS);
      Serial.print(". Connecting to ");
      Serial.print(m_mqttServer);
      Serial.print(":");
      Serial.print(m_mqttPort);
      Serial.print(" with user: ");
      Serial.print(m_mqttUsername);
      Serial.print(" and password: ");
      Serial.println(m_mqttPassword);
      Serial.print("    MQTT ID: ");
      Serial.println(mqttClientId);
      delay(10);
    }
    rp2040.wdt_reset();
    connected = m_mqttClient->connect(mqttClientId.c_str(),m_mqttUsername.c_str(), m_mqttPassword.c_str());
    rp2040.wdt_reset();
    if (connected) 
    {
      if (m_chattyCathy) Serial.println("    MQTT connected");
      m_mqttClient->subscribe(m_mqttSubscribeTopic.c_str());
//      m_mqttClient->publish(m_mqttPublishTopic.c_str(),"hello world");
 
      return;
    }
    else
    { 
      m_wifiStatus =  WiFi.status();
      if (m_wifiStatus != WL_CONNECTED)
      {
        if (m_chattyCathy) Serial.println("    Wifi disconnected. Restarting Wifi.");
        setup_wifi();
        return;
      }
      int mqttState = m_mqttClient->state();
      if (m_chattyCathy) 
      {
        Serial.print("    MQTT connection");
        Serial.print(" failed, rc=");
        Serial.print(mqttState);
        Serial.print(": ");
        switch (mqttState) 
        {
          case -4:
            Serial.println("MQTT_CONNECTION_TIMEOUT");
            break;
          case -3:
            Serial.println("MQTT_CONNECTION_LOST");
            break;
          case -2:
            Serial.println("MQTT_CONNECT_FAILED");
            break;
          case -1:
            Serial.println("MQTT_DISCONNECTED");
            break;
          case 0:
            Serial.println("MQTT_CONNECTED");
            break;
          case 1:
            Serial.println("MQTT_CONNECT_BAD_PROTOCOL");
            break;
          case 2:
            Serial.println("MQTT_CONNECT_BAD_CLIENT_ID");
            break;
          case 3:
            Serial.println("MQTT_CONNECT_UNAVAILABLE");
            break;
          case 4:
            Serial.println("MQTT_CONNECT_BAD_CREDENTIALS");
            break;
          case 5:
            Serial.println("MQTT_CONNECT_UNAUTHORIZED");
            break;
          default:
            break;
        }
        Serial.print("    Waiting ");
        Serial.print(MQTT_RETRY);
        Serial.print(" mS to retry.");
      }     
      int iwait = 0;
      while (iwait < MQTT_RETRY)
      {
        rp2040.wdt_reset();;
        delay(500);
        iwait = iwait + 500;
        if (m_chattyCathy) Serial.print(".");
        setCommLEDPin(!m_commLEDState);
        if (m_resetButtonPin > 0 )
        {
          if (digitalRead(m_resetButtonPin) == HIGH)
          {
            if (m_chattyCathy)
            {
              Serial.println("");
              Serial.println("Reset Button Pressed. ");
            }
            resetButtonPressed();
            return;
          }
        }
      }
      if (m_chattyCathy) Serial.println("");
      setCommLEDPin(false);
    }
  }
  if (m_chattyCathy)  Serial.println("    Too many MQTT attempts. Rebooting...");
  delay(1000);
  rp2040.reboot();
}
void mqttCallback(char* topic, byte* payload, unsigned int length) 
{
}
