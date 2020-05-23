/**
 * @file main.cpp
 * @brief Main file from Respirone project
 * @version 1.0
 */
#include <Arduino.h>
#include "user_interface.h"
#include "control.h"
#include "datalogger.h"
#include "power_supply.h"
#include "led_fw.h"
#include "time.h"

void setup()
{
  Control_Init();
  UI_Init();
  Time_Init();
  DataLogger_Init();
  PowerSupply_Init();
  LedInit();
}

void loop()
{
  Control_Task();
  UI_Task();
  DataLogger_Task();
  PowerSupply_Tasks();
  Time_Update_Task();
  LedTick();
} 