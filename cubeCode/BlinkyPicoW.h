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
#include <WiFi.h>
#include <LittleFS.h>
#include <PubSubClient.h>  

void   subscribeCubeData(char* topic, byte* payload, unsigned int length);

struct BlinkyPicoWDataHeader
{
  uint8_t  state;
  uint8_t  forceArchiveData;
  uint16_t watchdog;
  uint8_t  deviceMac[6];
  uint8_t  routerMac[6];
}; 

class BlinkyPicoW
{
  private:
    BlinkyPicoWDataHeader    m_mqttDataHeader;
    volatile boolean*        m_pcubeHasDataToRead;
    volatile boolean*        m_pmqttHasDataToRead;
    volatile boolean*        m_pforceArchiveData;
    volatile boolean*        m_pinitSettings;
     
    String          m_ssid = "ssid";
    String          m_wifiPassword = "wifiPassword";
    String          m_mqttServer = "mqttServer";
    String          m_mqttUsername = "mqttUsername";
    String          m_mqttPassword = "mqttPassword";
    String          m_box = "box";
    String          m_trayType = "trayType";
    String          m_trayName = "trayName";
    String          m_cubeType = "cube";

    String          m_mqttPublishTopic;
    String          m_mqttSubscribeTopic;

    boolean         m_init = true;
    uint32_t        m_hdwrWatchdogMs  = HDWR_WATCHDOG_MS;
    boolean         m_useFlashStorage = true;
    int             m_wifiStatus = 0;
    int             m_commLEDPin = LED_BUILTIN;
    boolean         m_commLEDState = false;
    boolean         m_chattyCathy = false;
    int             m_resetButtonPin = -1;
    int             m_resetButtonValue = LOW;
    unsigned long   m_resetButtonSwitchTime = 0;
    unsigned long   m_lastMqttPublishTime = 0;
    int             m_mqttLedFlashMs = MQTT_LED_FLASH_MS;

    boolean         m_wifiAccessPointMode = false;
    boolean         m_webPageServed = false;
    boolean         m_webPageRead = false;
    PubSubClient*   m_mqttClient;
    WiFiServer*     m_wifiServer;
    uint16_t        m_mqttKeepAlive = MQTT_KEEP_ALIVE;
    uint16_t        m_mqttSocketTimeout = MQTT_SOCKETTIMEOUT;
    int             m_mqttPort = MQTT_PORT;
         
    void            setup_wifi();
    void            setCommLEDPin(boolean ledState){m_commLEDState = ledState;digitalWrite(m_commLEDPin, m_commLEDState);};
    void            resetButtonPressed();
    void            readWebPage();
    void            serveWebPage();
    String          replaceHtmlEscapeChar(String inString);
    void            checkMqttConnection(); 

  public:
    BlinkyPicoW(boolean init, PubSubClient* mqttClient, volatile boolean* pcubeHasDataToRead, volatile boolean* pmqttHasDataToRead, volatile boolean* pforceArchiveData, volatile boolean* pinitSettings);
    void            loop();
    void            begin(int chattyCathy, int commLEDPin, int resetButtonPin, boolean useFlashStorage, size_t cubeSetting, size_t cubeReading);
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
    
    uint8_t*        m_pcubeDataSend = nullptr;
    uint8_t*        m_pcubeDataRecv = nullptr;
    size_t          m_sizeofCubeSetting;
    size_t          m_sizeofCubeReading;
    size_t          m_sizeofMqttDataHeader;
};
