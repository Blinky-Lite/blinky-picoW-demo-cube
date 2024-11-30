#define BLINKY_DIAG         1
#define COMM_LED_PIN       16
#define RST_BUTTON_PIN     15

#include "BlinkyPicoWCube.h"
struct CubeReading

{
  int16_t chipTemp;
};
struct CubeSetting
{
  int16_t led1;
  int16_t led2;
};
struct CubeData
{
  CubeReading reading;
  CubeSetting setting;
};
CubeData cubeData;

int led1Pin = 14;
int led2Pin = 17;

unsigned long lastPublishTime;

void setupBlinky()
{
  if (BLINKY_DIAG > 0) Serial.begin(9600);
  BlinkyPicoWCube.setSsid("georg");
  BlinkyPicoWCube.setWifiPassword("NONE");
  BlinkyPicoWCube.setMqttServer("192.168.4.1");
  BlinkyPicoWCube.setMqttUsername("blinky-lite-box");
  BlinkyPicoWCube.setMqttPassword("areallybadpassword");
  BlinkyPicoWCube.setBox("blinky-lite-box");
  BlinkyPicoWCube.setTrayType("blinky-picoW");
  BlinkyPicoWCube.setTrayName("picoW-02");
  BlinkyPicoWCube.setCubeType("cube");
  BlinkyPicoWCube.begin(BLINKY_DIAG, COMM_LED_PIN, RST_BUTTON_PIN, false);
}

void setupCube()
{
  pinMode(led1Pin, OUTPUT);
  pinMode(led2Pin, OUTPUT);
  cubeData.setting.led1 = 255;
  cubeData.setting.led2 = 0;
  analogWrite(led1Pin, cubeData.setting.led1);    
  analogWrite(led2Pin, cubeData.setting.led2);    
}
void loopCube()
{
  cubeData.reading.chipTemp = (int16_t) (analogReadTemp() * 100.0);
  delay(1000);
  if (cubeData.setting.led1 > 0) 
  {
    cubeData.setting.led1 = 0;
  }
  else
  {
    cubeData.setting.led1 = 255;
  }
  if (cubeData.setting.led2 > 0) 
  {
    cubeData.setting.led2 = 0;
  }
  else
  {
    cubeData.setting.led2 = 255;
  }
  analogWrite(led1Pin, cubeData.setting.led1);    
  analogWrite(led2Pin, cubeData.setting.led2);    
}
