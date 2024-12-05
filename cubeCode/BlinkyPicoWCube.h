#include "BlinkyPicoW.h"
WiFiClient g_wifiClient;
PubSubClient g_mqttClient(g_wifiClient);

bool core1_separate_stack = true;

//init to true to stop being used until memory is declared
volatile boolean g_cubeHasDataToRead = true;
volatile boolean g_mqttHasDataToRead = true;
volatile boolean g_forceArchiveData = false;
volatile boolean g_initSettings = true;

BlinkyPicoW BlinkyPicoWCube(true, &g_mqttClient, &g_cubeHasDataToRead, &g_mqttHasDataToRead, &g_forceArchiveData, &g_initSettings);
void setupBlinky();
void setupCube();
void loopCube();

void setup() 
{
  setupBlinky();
}
void loop()
{
  BlinkyPicoWCube.loop();
}

void setup1() 
{
  setupCube();
}
void loop1() 
{
  loopCube();
}
boolean publishCubeData(uint8_t* pcubeSetting, uint8_t* pcubeReading, boolean forceArchiveData)
{
  if (g_cubeHasDataToRead) return false;
  if (BlinkyPicoWCube.m_pcubeDataSend == nullptr) return false;

  uint8_t* memPtr = BlinkyPicoWCube.m_pcubeDataSend + BlinkyPicoWCube.m_sizeofMqttDataHeader;
  uint8_t* datPtr = pcubeSetting;
  for (int ii = 0; ii < BlinkyPicoWCube.m_sizeofCubeSetting; ++ii)
  {
    *memPtr = *datPtr;
    ++memPtr;
    ++datPtr;
  }
  datPtr = pcubeReading;
  for (int ii = 0; ii < BlinkyPicoWCube.m_sizeofCubeReading; ++ii)
  {
    *memPtr = *datPtr;
    ++memPtr;
    ++datPtr;
  }

  g_forceArchiveData = forceArchiveData;
  g_cubeHasDataToRead = true;
  return true;
}
void subscribeCubeData(char* topic, byte* payload, unsigned int length) 
{
  if (g_mqttHasDataToRead) return;
  unsigned int dataLength = BlinkyPicoWCube.m_sizeofMqttDataHeader + BlinkyPicoWCube.m_sizeofCubeSetting + BlinkyPicoWCube.m_sizeofCubeReading;
  if (dataLength != length) return;
  uint8_t* memPtr = BlinkyPicoWCube.m_pcubeDataRecv;
  uint8_t* datPtr = (uint8_t*) payload;
  for (int ii = 0; ii < dataLength; ++ii)
  {
    *memPtr = *datPtr;
    ++memPtr;
    ++datPtr;
  }
  g_mqttHasDataToRead = true;
  return;  
}
void retrieveCubeSetting(uint8_t* pcubeSetting)
{
  if (!g_mqttHasDataToRead) return;
  uint8_t* memPtr = BlinkyPicoWCube.m_pcubeDataRecv + BlinkyPicoWCube.m_sizeofMqttDataHeader;
  uint8_t* datPtr = pcubeSetting;
  for (int ii = 0; ii < BlinkyPicoWCube.m_sizeofCubeSetting; ++ii)
  {
    *datPtr = *memPtr;
    ++memPtr;
    ++datPtr;
  }
  g_mqttHasDataToRead = false;
  g_initSettings = false;
}
