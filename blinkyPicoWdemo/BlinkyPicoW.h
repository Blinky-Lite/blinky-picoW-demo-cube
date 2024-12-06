#ifndef BlinkyPicoW_h
#define BlinkyPicoW_h
bool core1_separate_stack = true;
#include "BlinkyPicoWMqtt.h"
WiFiClient g_wifiClient;
PubSubClient g_mqttClient(g_wifiClient);

BlinkyPicoWMqtt BlinkyPicoW(&g_mqttClient);
void setupBlinky();
void setupCube();
void loopCube();

void setup() 
{
  setupBlinky();
}
void loop()
{
  BlinkyPicoW.loop();
}

void setup1() 
{
  setupCube();
}
void loop1() 
{
  loopCube();
}
void mqttSubscribe(char* topic, byte* payload, unsigned int length)
{
  BlinkyPicoW.subscribeCubeData(topic, payload, length);
}

#endif
