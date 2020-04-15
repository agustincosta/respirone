#include <Arduino.h>
#include <Wire.h>
#include "user_interface.h"
#include "led_fw.h"


void setup()
{
  UI_Init();
  LedInit();
  // add init functions
}

void loop()
{
  LedTick();
  UI_Task();
  // add task functions
}


