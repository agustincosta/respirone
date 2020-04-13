#include <Arduino.h>
#include <Wire.h>
#include "user_interface.h"
#include "led_fw.h"


void setup()
{
  UserInterface_Init();
  LedInit();
  // add init functions
}

void loop()
{
  LedTick();
  UserInterface_Task();
  // add task functions
}


