#include "BlinkyMqttCube.h"

int commLEDPin = 16;
int commLEDBright = 255; 
int resetButtonPin = 15;
int led1Pin = 14;
int led2Pin = 17;

unsigned long lastPublishTime;
unsigned long publishInterval = 2000;

void setup1() 
{
//  Serial.begin(115200);
  lastPublishTime = millis();
  pinMode(led1Pin, OUTPUT);
  pinMode(led2Pin, OUTPUT);
  cubeData.state = 1;
  cubeData.watchdog = 0;
  cubeData.led1 = 0;
  cubeData.led2 = 0;
  analogWrite(led1Pin, cubeData.led1);    
  analogWrite(led2Pin, cubeData.led2);    
}
void setup() 
{
  // Optional setup to overide defaults
  BlinkyMqttCube.setChattyCathy(false);
  BlinkyMqttCube.setWifiTimeoutMs(20000);
  BlinkyMqttCube.setWifiRetryMs(20000);
  BlinkyMqttCube.setMqttRetryMs(3000);
  BlinkyMqttCube.setResetTimeoutMs(10000);
  BlinkyMqttCube.setHdwrWatchdogMs(8000);
  BlinkyMqttCube.setBlMqttKeepAlive(8);
  BlinkyMqttCube.setBlMqttSocketTimeout(6);
  BlinkyMqttCube.setMqttLedFlashMs(10);
  BlinkyMqttCube.setWirelesBlinkMs(100);
  BlinkyMqttCube.setMaxNoMqttErrors(5);
  
  // Must be included
  BlinkyMqttCube.init(commLEDPin, commLEDBright, resetButtonPin);
}

void loop1() 
{
  unsigned long nowTime = millis();
  BlinkyMqttCube::checkForSettings();
  
  if ((nowTime - lastPublishTime) > publishInterval)
  {
    lastPublishTime = nowTime;
    cubeData.watchdog = cubeData.watchdog + 1;
    if (cubeData.watchdog > 32760) cubeData.watchdog= 0 ;
    BlinkyMqttCube::publishToMqtt();
  }  
  
  cubeData.chipTemp = (int16_t) (analogReadTemp() * 100.0);
}
void loop() 
{
  BlinkyMqttCube.loop();
}

void handleNewMessage(uint8_t address)
{
  switch(address)
  {
    case 0:
      break;
    case 1:
      break;
    case 2:
      break;
    case 3:
      analogWrite(led1Pin, cubeData.led1);  
      break;
    case 4:
      analogWrite(led2Pin, cubeData.led2);    
      break;
    default:
      break;
  }
}
