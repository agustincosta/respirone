#include "motor.h"

MOTOR_t MOTOR;

Motor_States_e motorState; 

void Motor_Init()
{
  MOTOR.breathsMinute = 0;
  MOTOR.volumeMinute = 0;
}

void Motor_Tasks()
{
  switch (motorState)
  {
    case MOTOR_IDLE:
      motorState = MOTOR_RETURN_HOME_POSITION;
      motorState = MOTOR_VOLUME_CONTROL;
      motorState = MOTOR_PRESSURE_CONTROL;
      break;

    case MOTOR_RETURN_HOME_POSITION:
      //
      motorState = MOTOR_IDLE;
      break;    

    case MOTOR_VOLUME_CONTROL:
      //
      motorState = MOTOR_IDLE;
      break;    

    case MOTOR_PRESSURE_CONTROL:
      // PID
      motorState = MOTOR_IDLE;
      break;        
  
    default:
      break;
  }
}

void Motor_ReturnToHomePosition()
{

}

void Motor_VolumeModeSet(uint16_t tidalVolume, uint8_t breathsPerMinute, uint8_t IE_ratio)
{

}

void Motor_PressureModeSet(uint16_t adjustedPressure, uint8_t breatshPerMinute, uint8_t IE_ratio)
{

}

bool Motor_IsInHomePosition()
{
  return MOTOR.limitSwitch;
}

uint8_t Motor_GetBreathsPerMinute()
{
  return MOTOR.breathsMinute;
}

uint8_t Motor_GetVolumePerMinute()
{
  return MOTOR.volumeMinute;
}