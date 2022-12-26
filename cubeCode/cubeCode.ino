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
  Serial.begin(115200);
  lastPublishTime = millis();
  pinMode(led1Pin, OUTPUT);
  pinMode(led2Pin, OUTPUT);
  cubeData.state = 0;
  cubeData.watchdog = 0;
  cubeData.led1 = 50;
  cubeData.led2 = 100;
  setLeds();
}
void setup() 
{
  // Optional setup to overide defaults
  BlinkyMqttCube.setChattyCathy(true);
  BlinkyMqttCube.setWifiTimeoutMs(20000);
  BlinkyMqttCube.setWifiRetryMs(20000);
  BlinkyMqttCube.setMqttRetryMs(3000);
  BlinkyMqttCube.setResetTimeoutMs(10000);
  BlinkyMqttCube.setHdwrWatchdogMs(8000);
  BlinkyMqttCube.setBlMqttKeepAlive(8);
  BlinkyMqttCube.setBlMqttSocketTimeout(6);
  BlinkyMqttCube.setMqttLedFlashMs(10);
  BlinkyMqttCube.setWirelesBlinkMs(100);
  
  // Must be included
  BlinkyMqttCube.init(commLEDPin, commLEDBright, resetButtonPin);
}

void loop1() 
{
  unsigned long nowTime = millis();

  int fifoSize = rp2040.fifo.available();
  if (fifoSize > 0)
  {
    uint32_t command = 0;
    while (fifoSize > 0)
    {
      command = rp2040.fifo.pop();
      fifoSize = rp2040.fifo.available();
      delay(1);
    }
    switch (command) 
    {
      case 1:
        rp2040.fifo.push(command);
        fifoSize = 0;
        while (fifoSize == 0)
        {
          fifoSize = rp2040.fifo.available();
          delay(1);
        }
        command = rp2040.fifo.pop();
        break;
      default:
        // statements
        break;
    }
  }
 

  
  if ((nowTime - lastPublishTime) > publishInterval)
  {
    lastPublishTime = nowTime;
    cubeData.watchdog = cubeData.watchdog + 1;
    if (cubeData.watchdog > 32760) cubeData.watchdog= 0 ;
    uint32_t command = 1;
    rp2040.fifo.push(command);
    int fifoSize = 0;
    while (fifoSize == 0)
    {
      fifoSize = rp2040.fifo.available();
      delay(1);
    }
    command = rp2040.fifo.pop();
  }  
  
  cubeData.chipTemp = (int16_t) (analogReadTemp() * 100.0);
}
void loop() 
{
  BlinkyMqttCube.loop();
 //  publishBlinkyBusNow(); 
}



void setLeds()
{
  analogWrite(led1Pin, cubeData.led1);    
  analogWrite(led2Pin, cubeData.led2);    
}
