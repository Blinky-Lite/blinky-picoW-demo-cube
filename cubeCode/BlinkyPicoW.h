#define MAX_NO_CONNECTION_ATTEMPTS     5
#define STARTUP_DELAY               5000
#define WIFI_WAIT                  20000
#define DEBOUNCE_TIME                 20
#define RESET_TIMEOUT              10000
#include <WiFi.h>
#include <LittleFS.h>

//client.setNoDelay(true); Fix this!https://arduino-pico.readthedocs.io/en/latest/wificlient.html

class BlinkyPicoW
{
  private:
    String        g_ssid = "ssid";
    String        g_wifiPassword = "wifiPassword";
    String        g_mqttServer = "mqttServer";
    String        g_mqttUsername = "mqttUsername";
    String        g_mqttPassword = "mqttPassword";
    String        g_box = "box";
    String        g_trayType = "trayType";
    String        g_trayName = "trayName";
    String        g_cubeType = "cube";

    String        g_mqttPublishTopic;
    String        g_mqttSubscribeTopic;

    boolean       g_init = true;
    boolean       g_useFlashStorage = true;
    int           g_wifiStatus = 0;
    int           g_commLEDPin = LED_BUILTIN;
    boolean       g_commLEDState = false;
    boolean       g_chattyCathy = false;
    int           g_resetButtonPin = -1;
    int           g_resetButtonValue = LOW;
    unsigned long g_resetButtonSwitchTime = 0;
    boolean       g_wifiAccessPointMode = false;
    boolean       g_webPageServed = false;
    boolean       g_webPageRead = false;
    WiFiServer*   g_wifiServer;

    void          setup_wifi();
    void          setCommLEDPin(boolean ledState);
    void          resetButtonPressed();
    void          readWebPage();
    void          serveWebPage();
    String        replaceHtmlEscapeChar(String inString);

  public:
    BlinkyPicoW(boolean init);
    void          loop();
    void          begin(int chattyCathy, int commLEDPin, int resetButtonPin, boolean useFlashStorage);
    void          setSsid(String ssid);
    void          setWifiPassword(String wifiPassword);
    void          setMqttServer(String mqttServer);
    void          setMqttUsername(String mqttUsername);
    void          setMqttPassword(String mqttPassword);
    void          setBox(String box);
    void          setTrayType(String trayType);
    void          setTrayName(String trayName);
    void          setCubeType(String cubeType);

};
