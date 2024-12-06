#ifndef BlinkyPicoWMqtt_h
#define BlinkyPicoWMqtt_h

#include "Arduino.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <LittleFS.h>

#define MAX_NO_CONNECTION_ATTEMPTS     5
#define STARTUP_DELAY               5000
#define WIFI_WAIT                  20000
#define HDWR_WATCHDOG_MS            8000
#define DEBOUNCE_TIME                 20
#define RESET_TIMEOUT              10000
#define MQTT_LED_FLASH_MS            100
#define MQTT_KEEP_ALIVE               15
#define MQTT_SOCKETTIMEOUT             4
#define MQTT_PORT                   1883
#define MQTT_RETRY                  3000

struct BlinkyPicoWMqttDataHeader
{
  uint8_t  state;
  uint8_t  forceArchiveData;
  uint16_t watchdog;
  uint8_t  deviceMac[6];
  uint8_t  routerMac[6];
}; 
void mqttSubscribe(char* topic, byte* payload, unsigned int length);
class BlinkyPicoWMqtt
{
  private:
    boolean                 m_useFlashStorage = true;
    int                     m_wifiStatus = 0;
    int                     m_commLEDPin = LED_BUILTIN;
    boolean                 m_commLEDState = false;
    boolean                 m_chattyCathy = false;
    int                     m_resetButtonPin = -1;
    int                     m_resetButtonValue = LOW;
    PubSubClient*           m_pmqttClient;

    BlinkyPicoWMqttDataHeader   m_mqttDataHeader;
    volatile boolean        m_cubeHasDataToRead = true;
    volatile boolean        m_mqttHasDataToRead = true;
    volatile boolean        m_forceArchiveData = true;
    volatile boolean        m_initSettings = true;
    uint8_t*                m_pcubeDataSend = nullptr;
    uint8_t*                m_pcubeDataRecv = nullptr;
    size_t                  m_sizeofCubeSetting;
    size_t                  m_sizeofCubeReading;
    size_t                  m_sizeofMqttDataHeader;

    String                  m_ssid = "ssid";
    String                  m_wifiPassword = "wifiPassword";
    String                  m_mqttServer = "mqttServer";
    String                  m_mqttUsername = "mqttUsername";
    String                  m_mqttPassword = "mqttPassword";
    String                  m_box = "box";
    String                  m_trayType = "trayType";
    String                  m_trayName = "trayName";
    String                  m_cubeType = "cube";

    String                  m_mqttPublishTopic;
    String                  m_mqttSubscribeTopic;
    uint32_t                m_hdwrWatchdogMs  = HDWR_WATCHDOG_MS;
    unsigned long           m_resetButtonSwitchTime = 0;
    unsigned long           m_lastMqttPublishTime = 0;
    int                     m_mqttLedFlashMs = MQTT_LED_FLASH_MS;

    boolean                 m_wifiAccessPointMode = false;
    boolean                 m_webPageServed = false;
    boolean                 m_webPageRead = false;
    WiFiServer*             m_wifiServer;
    uint16_t                m_mqttKeepAlive = MQTT_KEEP_ALIVE;
    uint16_t                m_mqttSocketTimeout = MQTT_SOCKETTIMEOUT;
    int                     m_mqttPort = MQTT_PORT;
    
    void            setCommLEDPin(boolean ledState){m_commLEDState = ledState;digitalWrite(m_commLEDPin, m_commLEDState);};
    void            resetButtonPressed();
    void            setup_wifi();
    void            checkMqttConnection(); 
    void            readWebPage();
    void            serveWebPage();
    String          replaceHtmlEscapeChar(String inString);

  public:
    BlinkyPicoWMqtt(PubSubClient* pmqttClient);
    void            loop();
    void            begin(int chattyCathy, int commLEDPin, int resetButtonPin, boolean useFlashStorage, size_t cubeSetting, size_t cubeReading);
    boolean         publishCubeData(uint8_t* pcubeSetting, uint8_t* pcubeReading, boolean forceArchiveData);
    void            subscribeCubeData(char* topic, byte* payload, unsigned int length);
    boolean         retrieveCubeSetting(uint8_t* pcubeSetting);
    
    void            setSsid(String ssid){m_ssid = String(ssid);};
    void            setWifiPassword(String wifiPassword){m_wifiPassword = String(wifiPassword);};
    void            setMqttServer(String mqttServer){m_mqttServer = String(mqttServer);};
    void            setMqttUsername(String mqttUsername){m_mqttUsername = String(mqttUsername);};
    void            setMqttPassword(String mqttPassword){m_mqttPassword = String(mqttPassword);};
    void            setBox(String box){m_box = String(box);};
    void            setTrayType(String trayType){m_trayType = String(trayType);};
    void            setTrayName(String trayName){m_trayName = String(trayName);};
    void            setCubeType(String cubeType){m_cubeType = String(cubeType);};
    void            setMqttKeepAlive(uint16_t mqttKeepAlive){m_mqttKeepAlive = mqttKeepAlive;};
    void            setMqttSocketTimeout(uint16_t mqttSocketTimeout){m_mqttSocketTimeout = mqttSocketTimeout;};
    void            setMqttLedFlashMs(int mqttLedFlashMs){m_mqttLedFlashMs = mqttLedFlashMs;};
    void            setMqttPort(int mqttPort){m_mqttPort = mqttPort;};
    void            setHdwrWatchdogMs(uint32_t hdwrWatchdogMs){m_hdwrWatchdogMs = hdwrWatchdogMs;};
    
};


#endif
