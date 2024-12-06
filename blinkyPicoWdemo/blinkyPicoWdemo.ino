#define BLINKY_DIAG         1
#define COMM_LED_PIN       16
#define RST_BUTTON_PIN     15
#include "BlinkyPicoW.h"

struct CubeSetting
{
  uint8_t led1;
  uint8_t led2;
  uint16_t publishInterval;
};
CubeSetting setting;

struct CubeReading
{
  float chipTemp;
};
CubeReading reading;

int led1Pin = 14;
int led2Pin = 17;
int led1;
int led2;
int signLed1;
int signLed2;

unsigned long lastPublishTime;

void setupBlinky()
{
  if (BLINKY_DIAG > 0) Serial.begin(9600);

  BlinkyPicoW.setSsid("georg");
  BlinkyPicoW.setWifiPassword("NONE");
  BlinkyPicoW.setMqttServer("192.168.4.1");
  BlinkyPicoW.setMqttUsername("blinky-lite-box-01");
  BlinkyPicoW.setMqttPassword("areallybadpassword");
  BlinkyPicoW.setBox("blinky-lite-box-01");
  BlinkyPicoW.setTrayType("blinky-picoW");
  BlinkyPicoW.setTrayName("picoW-02");
  BlinkyPicoW.setCubeType("cube");
  BlinkyPicoW.setMqttKeepAlive(15);
  BlinkyPicoW.setMqttSocketTimeout(4);
  BlinkyPicoW.setMqttPort(1883);
  BlinkyPicoW.setMqttLedFlashMs(100);
  BlinkyPicoW.setHdwrWatchdogMs(8000);

  BlinkyPicoW.begin(BLINKY_DIAG, COMM_LED_PIN, RST_BUTTON_PIN, true, sizeof(setting), sizeof(reading));
}

void setupCube()
{
  pinMode(led1Pin, OUTPUT);
  pinMode(led2Pin, OUTPUT);
  setting.led1 = 255;
  setting.led2 = 255;
  setting.publishInterval = 3000;

  led1 = 0;
  led2 = 255;
  signLed1 = 1;
  signLed2 = -1;
  analogWrite(led1Pin, led1);    
  analogWrite(led2Pin, led2);   
  lastPublishTime = millis(); 
}
void loopCube()
{
  unsigned long now = millis();
  if ((now - lastPublishTime) > setting.publishInterval)
  {
    lastPublishTime = now;
    reading.chipTemp = analogReadTemp();
    boolean successful = BlinkyPicoW.publishCubeData((uint8_t*) &setting, (uint8_t*) &reading, false);
  }

  led1 = led1 + signLed1;    
  led2 = led2 + signLed2;    
  analogWrite(led1Pin, led1);    
  analogWrite(led2Pin, led2);

  if (led1 >= (int) setting.led1) signLed1 = -1;
  if (led2 >= (int) setting.led2) signLed2 = -1;
  if (led1 == 0) signLed1 = 1;
  if (led2 == 0) signLed2 = 1;
  delay(5);
  boolean newSettings = BlinkyPicoW.retrieveCubeSetting((uint8_t*) &setting);
}
