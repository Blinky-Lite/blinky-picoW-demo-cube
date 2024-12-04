#define BLINKY_DIAG         1
#define COMM_LED_PIN       16
#define RST_BUTTON_PIN     15

#include "BlinkyPicoWCube.h"

struct CubeSetting
{
  uint8_t led1;
  uint8_t led2;
  uint16_t publishInterval;
};
struct CubeReading
{
  int16_t chipTemp;
};
struct CubeData
{
  CubeSetting setting;
  CubeReading reading;
};
CubeData cubeData;

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
  BlinkyPicoWCube.setSsid("georg");
  BlinkyPicoWCube.setWifiPassword("NONE");
  BlinkyPicoWCube.setMqttServer("192.168.4.1");
  BlinkyPicoWCube.setMqttUsername("blinky-lite-box-01");
  BlinkyPicoWCube.setMqttPassword("areallybadpassword");
  BlinkyPicoWCube.setBox("blinky-lite-box-01");
  BlinkyPicoWCube.setTrayType("blinky-picoW");
  BlinkyPicoWCube.setTrayName("picoW-02");
  BlinkyPicoWCube.setCubeType("cube");
  BlinkyPicoWCube.setMqttKeepAlive(15);
  BlinkyPicoWCube.setMqttSocketTimeout(4);
  BlinkyPicoWCube.setMqttPort(1883);
  BlinkyPicoWCube.setMqttLedFlashMs(100);
  BlinkyPicoWCube.setHdwrWatchdogMs(8000);
  BlinkyPicoWCube.begin(BLINKY_DIAG, COMM_LED_PIN, RST_BUTTON_PIN, true, sizeof(cubeData));
}

void setupCube()
{
  pinMode(led1Pin, OUTPUT);
  pinMode(led2Pin, OUTPUT);
  cubeData.setting.led1 = 255;
  cubeData.setting.led2 = 255;
  cubeData.setting.publishInterval = 30000;

  led1 = 0;
  led2 = 0;
  signLed1 = 255;
  signLed2 = 255;
  analogWrite(led1Pin, led1);    
  analogWrite(led2Pin, led2);   
  lastPublishTime = millis(); 
}
void loopCube()
{
  unsigned long now = millis();
  if ((now - lastPublishTime) > cubeData.setting.publishInterval)
  {
    lastPublishTime = now;
    cubeData.reading.chipTemp = (int16_t) (analogReadTemp() * 100.0);

    boolean successful = publishCubeData((uint8_t*) &cubeData, false);
  }

  led1 = led1 + signLed1;    
  led2 = led2 + signLed2;    
  analogWrite(led1Pin, led1);    
  analogWrite(led2Pin, led2);

  if (led1 == (int) cubeData.setting.led1) signLed1 = -1;
  if (led2 == (int) cubeData.setting.led2) signLed2 = -1;
  if (led1 == 0) signLed1 = 1;
  if (led2 == 0) signLed2 = 1;
  delay(5);

}
