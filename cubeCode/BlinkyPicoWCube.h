#include "BlinkyPicoW.h"
BlinkyPicoW BlinkyPicoWCube(true);
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
