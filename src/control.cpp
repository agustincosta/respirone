/**
 * @file control.cpp
 * @brief Control module
 * @version 0.1
 */
#include "control.h"
#include "sensor.h"
#include "motor.h"

CTRL_t CTRL;

void Control_Init()
{
  Sensor_Init();
  Motor_Init();
}

void Control_Task()
{
  Sensor_Tasks();
  Motor_Tasks();
}