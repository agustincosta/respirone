/**
 * @file main.cpp
 * @brief Main file from Respirone project
 * @version 1.0
 */
#include <Arduino.h>
#include "user_interface.h"
#include "control.h"
#include "datalogger.h"
#include "led_fw.h"

void setup()
{
  UI_Init();
  Control_Init();
  DataLogger_Init();
  LedInit();
}

void loop()
{
  Control_Task();
  UI_Task();
  DataLogger_Task();
  LedTick();
}