#include "BlinkyPicoW.h"

BlinkyPicoW::BlinkyPicoW(boolean init)
{
  g_init = init;
  g_resetButtonValue = LOW;
  g_resetButtonSwitchTime = 0;
  g_wifiAccessPointMode = false;
  g_webPageServed = false;
  g_webPageRead = false;
}
void BlinkyPicoW::begin(int chattyCathy, int commLEDPin, int resetButtonPin, boolean useFlashStorage)
{
  g_chattyCathy = false;
  if (chattyCathy > 0) g_chattyCathy = true;
  g_commLEDPin = commLEDPin;
  pinMode(g_commLEDPin, OUTPUT);
  setCommLEDPin(false);
  g_resetButtonPin = resetButtonPin;
  if (g_resetButtonPin > 0 )
  { 
    pinMode(g_resetButtonPin, INPUT);
  }
  g_useFlashStorage = useFlashStorage;
  int idot = 0;
  while (idot < STARTUP_DELAY)
  {
    delay(50);
    idot = idot + 50;
    setCommLEDPin(!g_commLEDState);
  }
  setCommLEDPin(false);
  if (g_chattyCathy) Serial.println("");
  if (g_chattyCathy) Serial.println("Beginning BlinkyPicoWCube");
  if (g_useFlashStorage)
  {
    if (g_chattyCathy) Serial.println("Reading creds.txt file");
    if (g_chattyCathy) Serial.println("    Data:");
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
          if (g_chattyCathy)
          {
            Serial.print("        ");
            Serial.println(lines[ii]);
          }
          data = data.substring(stopPos + 1);
        }
        g_ssid               = lines[0];
        g_wifiPassword       = lines[1];
        g_mqttServer         = lines[2];
        g_mqttUsername       = lines[3];
        g_mqttPassword       = lines[4];
        g_box                = lines[5];
        g_trayType           = lines[6];
        g_trayName           = lines[7];
        g_cubeType           = lines[8];
        file.close();
    }
    else
    {
      if (g_chattyCathy) Serial.println("file open failed");
    }
  }
  g_mqttSubscribeTopic = g_box + "/" + g_cubeType + "/" + g_trayType + "/" + g_trayName + "/setting";
  g_mqttPublishTopic   = g_box + "/" + g_cubeType + "/" + g_trayType + "/" + g_trayName + "/reading";
  if (g_chattyCathy) Serial.println("    Variables:");
  if (g_chattyCathy) Serial.print("        g_ssid:             ");
  if (g_chattyCathy) Serial.println(g_ssid);
  if (g_chattyCathy) Serial.print("        wifiPassword:       ");
  if (g_chattyCathy) Serial.println(g_wifiPassword);
  if (g_chattyCathy) Serial.print("        mqttServer:         ");
  if (g_chattyCathy) Serial.println(g_mqttServer);
  if (g_chattyCathy) Serial.print("        mqttUsername:       ");
  if (g_chattyCathy) Serial.println(g_mqttUsername);
  if (g_chattyCathy) Serial.print("        mqttPassword:       ");
  if (g_chattyCathy) Serial.println(g_mqttPassword);
  if (g_chattyCathy) Serial.print("        trayType:           ");
  if (g_chattyCathy) Serial.println(g_trayType);
  if (g_chattyCathy) Serial.print("        trayName:           ");
  if (g_chattyCathy) Serial.println(g_trayName);
  if (g_chattyCathy) Serial.print("        cubeType            ");
  if (g_chattyCathy) Serial.println(g_cubeType);
  if (g_chattyCathy) Serial.print("        mqttPublishTopic:   ");
  if (g_chattyCathy) Serial.println(g_mqttPublishTopic);
  if (g_chattyCathy) Serial.print("        mqttSubscribeTopic: ");
  if (g_chattyCathy) Serial.println(g_mqttSubscribeTopic);
  if (g_chattyCathy) Serial.println("");
  setup_wifi();
  
  return;
}

void BlinkyPicoW::loop()
{
  unsigned long nowTime = millis();
  if (g_wifiAccessPointMode)
  {
    serveWebPage();
    readWebPage();
    delay(100);
    return;
  }
// Check reset button
  if (g_resetButtonPin > 0 )
  {
    int pinValue = digitalRead(g_resetButtonPin);
    if (pinValue != g_resetButtonValue)
    {
      if((nowTime - g_resetButtonSwitchTime) > DEBOUNCE_TIME)
      {
        g_resetButtonValue = pinValue;
        g_resetButtonSwitchTime = nowTime;
        if (g_chattyCathy)
        {
          if (g_resetButtonValue == HIGH)
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
    if (g_resetButtonValue == HIGH)
    {
      if((nowTime - g_resetButtonSwitchTime) > RESET_TIMEOUT)
      {
        if (g_chattyCathy)
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
  g_wifiStatus =  WiFi.status();
  if (g_wifiStatus != WL_CONNECTED)
  {
    if (g_chattyCathy) Serial.println("Wifi disconnected. Restarting Wifi.");
    setup_wifi();
    return;
  }
  else
  {
    delay(1);
  }
  return;
}
void BlinkyPicoW::setup_wifi() 
{
  if (g_chattyCathy) Serial.println();
  if (g_chattyCathy) Serial.println("Starting Communications");
  setCommLEDPin(true);
  WiFi.mode(WIFI_STA); //This will non-block the wifi begin
  int itry = 0;
  while (itry < MAX_NO_CONNECTION_ATTEMPTS)
  {
    itry = itry + 1;
    if (g_chattyCathy) Serial.print("    Attempt: ");
    if (g_chattyCathy) Serial.print(itry);
    if (g_chattyCathy) Serial.print("/");
    if (g_chattyCathy) Serial.print(MAX_NO_CONNECTION_ATTEMPTS);
    if (g_chattyCathy) Serial.print(". Connecting to ");
    if (g_chattyCathy) Serial.print(g_ssid);
    if (g_chattyCathy) Serial.print(" with password: ");
    if (g_chattyCathy) Serial.println(g_wifiPassword);
  
    if (g_wifiPassword.equals("NONE"))
    {
      if (g_chattyCathy ) Serial.println("    Connecting with no password");
      WiFi.begin(g_ssid.c_str());
    }
    else
    {
      WiFi.begin(g_ssid.c_str(), g_wifiPassword.c_str());
    }
    setCommLEDPin(false);
    if (g_chattyCathy) 
    {
      Serial.print("    Waiting ");
      Serial.print(WIFI_WAIT);
      Serial.print(" mS for wifi.");
    }
    int iwait = 0;
    while (iwait < WIFI_WAIT)
    {
      delay(500);
      iwait = iwait + 500;
      if (g_chattyCathy) Serial.print(".");
      setCommLEDPin(!g_commLEDState);
      if (g_resetButtonPin > 0 )
      {
        if (digitalRead(g_resetButtonPin) == HIGH)
        {
          if (g_chattyCathy) Serial.println("");
          if (g_chattyCathy) Serial.println("Reset Button Pressed. ");
          resetButtonPressed();
          return;
        }
      }
    }
    if (g_chattyCathy) Serial.println("");
    setCommLEDPin(false);
    g_wifiStatus =  WiFi.status();
    if (g_chattyCathy )
    {
      if (g_wifiStatus != WL_CONNECTED)
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
  
  if (g_chattyCathy) Serial.println("");
  setCommLEDPin(false);
  if ( g_wifiStatus == WL_CONNECTED)
  {
    if (g_chattyCathy) Serial.print("    WiFi connected with IP address: ");
    if (g_chattyCathy) Serial.println(WiFi.localIP());
    if (g_chattyCathy)
    {
      byte mac[6];
      WiFi.macAddress(mac);
      Serial.print("    MAC: ");
      Serial.print(mac[0],HEX);
      Serial.print(":");
      Serial.print(mac[1],HEX);
      Serial.print(":");
      Serial.print(mac[2],HEX);
      Serial.print(":");
      Serial.print(mac[3],HEX);
      Serial.print(":");
      Serial.print(mac[4],HEX);
      Serial.print(":");
      Serial.println(mac[5],HEX);
      Serial.println("");
    }
    g_init = false;
  }
  else
  {
    if (g_chattyCathy)  Serial.println("Too many wifi attempts. Rebooting...");
    delay(1000);
    rp2040.reboot();
  }

}
void BlinkyPicoW::resetButtonPressed()
{
  if (g_wifiAccessPointMode) return;
  if (!g_useFlashStorage)
  {
    if (g_chattyCathy)  Serial.println("Rebooting...");
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

  WiFi.softAP(g_trayType + "-" + g_trayName);
  if (g_chattyCathy) Serial.print("Access Point Created with IP Gateway ");
  if (g_chattyCathy) Serial.println(WiFi.softAPIP());
  g_wifiServer = new WiFiServer(80);
  g_wifiAccessPointMode = true;
  g_wifiServer->begin();
//  if (g_chattyCathy) Serial.print("    IP address:");
//  IPAddress myAddress = WiFi.localIP();
//  if (g_chattyCathy) Serial.println(myAddress);
  g_webPageServed = false;
  g_webPageRead = false;
}
void BlinkyPicoW::readWebPage()
{
  if (!g_webPageServed) return;
  if (g_webPageRead) return;
  WiFiClient client = g_wifiServer->accept();

  if (client) 
  {
    String header = "";
    while (client.connected() && !g_webPageRead) 
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
      g_ssid               = replaceHtmlEscapeChar(data.substring(5,data.indexOf("&pass=")));
      g_wifiPassword       = replaceHtmlEscapeChar(data.substring(data.indexOf("&pass=")  + 6,data.indexOf("&serv=")));
      g_mqttServer         = replaceHtmlEscapeChar(data.substring(data.indexOf("&serv=")  + 6,data.indexOf("&unam=")));
      g_mqttUsername       = replaceHtmlEscapeChar(data.substring(data.indexOf("&unam=")  + 6,data.indexOf("&mpas=")));
      g_mqttPassword       = replaceHtmlEscapeChar(data.substring(data.indexOf("&mpas=")  + 6,data.indexOf("&bbox=")));
      g_box                = replaceHtmlEscapeChar(data.substring(data.indexOf("&bbox=")  + 6,data.indexOf("&tryt=")));
      g_trayType           = replaceHtmlEscapeChar(data.substring(data.indexOf("&tryt=")  + 6,data.indexOf("&tryn=")));
      g_trayName           = replaceHtmlEscapeChar(data.substring(data.indexOf("&tryn=")  + 6,data.indexOf("&cube=")));
      g_cubeType           = replaceHtmlEscapeChar(data.substring(data.indexOf("&cube=")  + 6,data.length()));
      g_mqttSubscribeTopic = g_box + "/" + g_cubeType + "/" + g_trayType + "/" + g_trayName + "/setting";
      g_mqttPublishTopic   = g_box + "/" + g_cubeType + "/" + g_trayType + "/" + g_trayName + "/reading";
      if (g_chattyCathy) Serial.println("    Reading returned web form.");
      if (g_chattyCathy) Serial.println("");
      if (g_chattyCathy) Serial.print("    g_ssid:             ");
      if (g_chattyCathy) Serial.println(g_ssid);
      if (g_chattyCathy) Serial.print("    wifiPassword:       ");
      if (g_chattyCathy) Serial.println(g_wifiPassword);
      if (g_chattyCathy) Serial.print("    mqttServer:         ");
      if (g_chattyCathy) Serial.println(g_mqttServer);
      if (g_chattyCathy) Serial.print("    mqttUsername:       ");
      if (g_chattyCathy) Serial.println(g_mqttUsername);
      if (g_chattyCathy) Serial.print("    mqttPassword:       ");
      if (g_chattyCathy) Serial.println(g_mqttPassword);
      if (g_chattyCathy) Serial.print("    trayType:           ");
      if (g_chattyCathy) Serial.println(g_trayType);
      if (g_chattyCathy) Serial.print("    trayName:           ");
      if (g_chattyCathy) Serial.println(g_trayName);
      if (g_chattyCathy) Serial.print("    cubeType            ");
      if (g_chattyCathy) Serial.println(g_cubeType);
      if (g_chattyCathy) Serial.print("    mqttPublishTopic:   ");
      if (g_chattyCathy) Serial.println(g_mqttPublishTopic);
      if (g_chattyCathy) Serial.print("    mqttSubscribeTopic: ");
      if (g_chattyCathy) Serial.println(g_mqttSubscribeTopic);
      if (g_chattyCathy) Serial.println("");
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
      g_webPageRead = true;
    }
    delay(100);
    client.stop();
    if (g_webPageRead)
    {
      g_wifiAccessPointMode = false;
      WiFi.disconnect();
      if (g_chattyCathy) Serial.println("    Writing creds.txt file");
      File file = LittleFS.open("/creds.txt", "w");
      if (file) 
      {
        String line = "";
        line = "{" + g_ssid + "}";
        file.println(line);
        line = "{" + g_wifiPassword + "}";
        file.println(line);
        line = "{" + g_mqttServer + "}";
        file.println(line);
        line = "{" + g_mqttUsername + "}";
        file.println(line);
        line = "{" + g_mqttPassword + "}";
        file.println(line);
        line = "{" + g_box + "}";
        file.println(line);
        line = "{" + g_trayType + "}";
        file.println(line);
        line = "{" + g_trayName + "}";
        file.println(line);
        line = "{" + g_cubeType + "}";
        file.println(line);
        file.println("");
        file.println("");
        file.println("");
        file.close();
      }
      else
      {
        if (g_chattyCathy) Serial.println("    File open failed");
      }
      if (g_chattyCathy) Serial.println("    File writing complete.");
      if (g_chattyCathy) Serial.println("    Rebooting");
      rp2040.reboot();

    }
  }
}
void BlinkyPicoW::serveWebPage()
{
  if (g_webPageServed) return;
  WiFiClient client = g_wifiServer->accept();

  if (client) 
  {
    if (g_chattyCathy) Serial.println("    new client. Serving webpage.");
    if (g_chattyCathy) Serial.println("");
    String header = "";
    while (client.connected() && !g_webPageServed) 
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
//    if (g_chattyCathy) Serial.print("    Returned header: ");
//    if (g_chattyCathy) Serial.println(header);
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
    String tag = "<td class=\"cell\"><input name=\"ssid\" id=\"ssid\" type=\"text\" value=\"" + g_ssid + "\" class=\"formtext\"/></td>";
    client.println(tag.c_str());
    client.println("</tr>");
    
    client.println("<tr>");
    client.println("<td class=\"cell\"><label for=\"pass\" class=\"labeltext\">Wifi Password</label></td>");
    tag = "<td class=\"cell\"><input name=\"pass\" id=\"pass\" type=\"password\" value=\"" + g_wifiPassword + "\" class=\"formtext\"/></td>";
    client.println(tag.c_str());
    client.println("</tr>");
    
    client.println("<tr>");
    client.println("<td class=\"cell\"><label for=\"serv\" class=\"labeltext\">MQTT Server</label></td>");
    tag = "<td class=\"cell\"><input name=\"serv\" id=\"serv\" type=\"text\" value=\"" + g_mqttServer + "\" class=\"formtext\"/></td>";
    client.println(tag.c_str());
    client.println("</tr>");
    
    client.println("<tr>");
    client.println("<td class=\"cell\"><label for=\"unam\" class=\"labeltext\">MQTT Username</label></td>");
    tag = "<td class=\"cell\"><input name=\"unam\" id=\"unam\" type=\"text\" value=\"" + g_mqttUsername + "\" class=\"formtext\"/></td>";
    client.println(tag.c_str());
    client.println("</tr>");
    
    client.println("<tr>");
    client.println("<td class=\"cell\"><label for=\"mpas\" class=\"labeltext\">MQTT Password</label></td>");
    tag = "<td class=\"cell\"><input name=\"mpas\" id=\"mpas\" type=\"password\" value=\"" + g_mqttPassword + "\" class=\"formtext\"/></td>";
    client.println(tag.c_str());
    client.println("</tr>");
   
    client.println("<tr>");
    client.println("<td class=\"cell\"><label for=\"bbox\" class=\"labeltext\">Box</label></td>");
    tag = "<td class=\"cell\"><input name=\"bbox\" id=\"bbox\" type=\"text\" value=\"" + g_box + "\" class=\"formtext\"/></td>";
    client.println(tag.c_str());
    client.println("</tr>");

    client.println("<tr>");
    client.println("<td class=\"cell\"><label for=\"tryt\" class=\"labeltext\">Tray Type</label></td>");
    tag = "<td class=\"cell\"><input name=\"tryt\" id=\"tryt\" type=\"text\" value=\"" + g_trayType + "\" class=\"formtext\"/></td>";
    client.println(tag.c_str());
    client.println("</tr>");
      
    client.println("<tr>");
    client.println("<td class=\"cell\"><label for=\"tryn\" class=\"labeltext\">Tray Name</label></td>");
    tag = "<td class=\"cell\"><input name=\"tryn\" id=\"tryn\" type=\"text\" value=\"" + g_trayName + "\" class=\"formtext\"/></td>";
    client.println(tag.c_str());
    client.println("</tr>");
      
    client.println("<tr>");
    client.println("<td class=\"cell\"><label for=\"cube\" class=\"labeltext\">Cube Type</label></td>");
    tag = "<td class=\"cell\"><input name=\"cube\" id=\"cube\" type=\"text\" value=\"" + g_cubeType + "\" class=\"formtext\"/></td>";
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
    g_webPageRead = false;
    g_webPageServed = true;
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
void BlinkyPicoW::setCommLEDPin(boolean ledState)
{
  g_commLEDState = ledState;
  digitalWrite(g_commLEDPin, g_commLEDState);
}
void BlinkyPicoW::setSsid(String ssid)
{
  g_ssid = String(ssid);
}
void BlinkyPicoW::setWifiPassword(String wifiPassword)
{
  g_wifiPassword = String(wifiPassword);
}
void BlinkyPicoW::setMqttServer(String mqttServer)
{
  g_mqttServer = String(mqttServer);
}
void BlinkyPicoW::setMqttUsername(String mqttUsername)
{
  g_mqttUsername = String(mqttUsername);
}
void BlinkyPicoW::setMqttPassword(String mqttPassword)
{
  g_mqttPassword = String(mqttPassword);
}
void BlinkyPicoW::setBox(String box)
{
  g_box = String(box);
}
void BlinkyPicoW::setTrayType(String trayType)
{
  g_trayType = String(trayType);
}
void BlinkyPicoW::setTrayName(String trayName)
{
  g_trayName = String(trayName);
}
void BlinkyPicoW::setCubeType(String cubeType)
{
  g_cubeType = String(cubeType);
}
