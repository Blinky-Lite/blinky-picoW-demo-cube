union CubeData
{
  struct
  {
    int16_t state;
    int16_t watchdog;
    int16_t chipTemp;
    int16_t led1;
    int16_t led2;
  };
  byte buffer[10];
};
CubeData cubeData;
