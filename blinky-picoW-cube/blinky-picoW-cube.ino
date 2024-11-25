boolean printDiagnostics = false;

union CubeData
{
  struct
  {
    int16_t state;
    int16_t watchdog;
    int16_t newData;
    int16_t chipTemp;
    int16_t led1;
    int16_t led2;
  };
  byte buffer[12];
};
CubeData cubeData;

#include "BlinkyPicoWCube.h"


int commLEDPin = 16;
int commLEDBright = 255; 
int resetButtonPin = 15;
int led1Pin = 14;
int led2Pin = 17;

unsigned long lastPublishTime;
unsigned long publishInterval = 2000;

void setupServerComm()
{
  // Optional setup to overide defaults
  if (printDiagnostics) Serial.begin(115200);
  delay(5000);
  BlinkyPicoWCube.setChattyCathy(printDiagnostics);
  BlinkyPicoWCube.setWifiTimeoutMs(20000);
  BlinkyPicoWCube.setWifiRetryMs(20000);
  BlinkyPicoWCube.setMqttRetryMs(3000);
  BlinkyPicoWCube.setResetTimeoutMs(10000);
  BlinkyPicoWCube.setHdwrWatchdogMs(8000);
  BlinkyPicoWCube.setBlMqttKeepAlive(8);
  BlinkyPicoWCube.setBlMqttSocketTimeout(4);
  BlinkyPicoWCube.setMqttLedFlashMs(10);
  BlinkyPicoWCube.setWirelesBlinkMs(100);
  BlinkyPicoWCube.setMaxNoMqttErrors(5);
  BlinkyPicoWCube.setMaxNoConnectionAttempts(5);
  
  // Must be included
  BlinkyPicoWCube.init(commLEDPin, commLEDBright, resetButtonPin);
}

void setupCube()
{
  lastPublishTime = millis();
  pinMode(led1Pin, OUTPUT);
  pinMode(led2Pin, OUTPUT);
  cubeData.state = 1;
  cubeData.watchdog = 0;
  cubeData.newData = 0;
  cubeData.led1 = 0;
  cubeData.led2 = 0;
  analogWrite(led1Pin, cubeData.led1);    
  analogWrite(led2Pin, cubeData.led2);    
}

void cubeLoop()
{
  unsigned long nowTime = millis();
  checkNewData(nowTime);
  
  if ((nowTime - lastPublishTime) > publishInterval)
  {
    lastPublishTime = nowTime;
    cubeData.watchdog = cubeData.watchdog + 1;
    if (cubeData.watchdog > 32760) cubeData.watchdog= 0 ;
    BlinkyPicoWCube.publishToServer();
  }  
  
  cubeData.chipTemp = (int16_t) (analogReadTemp() * 100.0);
}
void checkNewData(unsigned long nowTime)
{
  if (cubeData.newData == 0 ) return;
  lastPublishTime = nowTime;
  cubeData.watchdog = cubeData.watchdog + 1;
  if (cubeData.watchdog > 32760) cubeData.watchdog= 0 ;
  BlinkyPicoWCube.publishToServer();
  BlinkyPicoWCube.loop();
  cubeData.newData = 0;
}

void handleNewSettingFromServer(uint8_t address)
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
      break;
    case 4:
      analogWrite(led1Pin, cubeData.led1); 
      cubeData.newData = 1; 
      break;
    case 5:
      analogWrite(led2Pin, cubeData.led2);  
      cubeData.newData = 1; 
      break;
    default:
      break;
  }
}
