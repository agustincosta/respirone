/**
 * @file control.cpp
 * @brief Control module
 * @version 0.1
 */
#include "control.h"
#include "sensor.h"
#include "motor.h"
#include "user_interface.h"
#include "datalogger.h"

CTRL_t CTRL;

Control_Modes_e controlMode;
Control_VolumeStates_e controlVolumeState;
Control_PressureStates_e controlPressureState;

void Control_Init()
{
  Sensor_Init();
  Motor_Init();

  controlMode = CONTROL_IDLE;
  controlVolumeState = CTRL_VOLUME_INIT;
  controlPressureState = CTRL_PRESSURE_INIT;
}

void Control_Task()
{
  switch (controlMode)
  {
    case CONTROL_IDLE:
      if (UI.setUpComplete)
      {
        UI.setUpComplete = false;

        switch (UI.selectedMode)
        {
          case UI_VOLUME_CONTROL:
            controlMode = VOLUME_CONTROL;
            break;

          case UI_PRESSURE_CONTROL:
            controlMode = PRESSURE_CONTROL;
            break;    
        
          default:
            // ToDo: report error
            break;
        }
      }
      break; 

    case VOLUME_CONTROL:
      if (UI.setUpComplete)
      {
        UI.setUpComplete = false;

        if (UI.selectedMode==UI_PRESSURE_CONTROL)
        {
          controlMode = PRESSURE_CONTROL;
          break; 
        }         
      }
      else
      {
        Control_VolumeModeTask();
      }
      break;

    case PRESSURE_CONTROL:
      if (UI.setUpComplete)
      {
        UI.setUpComplete = false;

        if (UI.selectedMode==UI_VOLUME_CONTROL)
        {
          controlMode = VOLUME_CONTROL;
        }         
      }
      else
      {
        Control_PressureModeTask();
      }

      break;      
  
    default:
      // ToDo: report error
      break;
  }
}

void Control_VolumeModeTask()
{
  static uint16_t inspirationTimeout, expirationTimeout;
  
  switch (controlVolumeState)
  {
    case CTRL_VOLUME_INIT:
      //ToDo> motor return to home position  

      controlVolumeState = CTRL_VOLUME_INSPIRATION_SETUP;

      inspirationTimeout = MINUTE_MS/(UI.breathPerMinute*(UI.IE_ratio+1));
      expirationTimeout  = (MINUTE_MS*UI.IE_ratio)/(UI.breathPerMinute*(UI.IE_ratio+1));   

      break;

    case CTRL_VOLUME_INSPIRATION_SETUP:
      controlVolumeState = CTRL_VOLUME_INSPIRATION_MONITORING;

      // ToDo> set motor speed
      // Motor_Setpoint(UI.TIDALvolume, UI.breathPerMinute, UI.IE_ratio);

      CTRL_Timer(0);
      break;

    case CTRL_VOLUME_INSPIRATION_MONITORING:

      // check Pval<Pmax else alarm
      // Sensor_GetValue()>UI.pressureMax alarm

      if(CTRL_Timer(inspirationTimeout))
      {
        // update volume/minute
        controlVolumeState = CTRL_VOLUME_EXPIRATION_SETUP;
      }
      break;

    case CTRL_VOLUME_EXPIRATION_SETUP:
      controlVolumeState = CTRL_VOLUME_EXPIRATION_IDLE;

      // ToDo> set motor speed

      CTRL_Timer(0);
      break;

    case CTRL_VOLUME_EXPIRATION_IDLE:
      // ToDo> check limit switch

      if(CTRL_Timer(expirationTimeout))
      {    
        // update breath per minute
        controlVolumeState = CTRL_VOLUME_INSPIRATION_SETUP;
      }
      // else inspiration 
      break;      

    default:
      controlVolumeState = CTRL_VOLUME_INIT;
      // ToDo: report error
      break;
  }
}

void Control_PressureModeTask()
{
  switch (controlPressureState)
  {
    case CTRL_PRESSURE_INIT:
      controlPressureState = CTRL_PRESSURE_INSPIRATION_SETUP;
      break;

    case CTRL_PRESSURE_INSPIRATION_SETUP:
      controlPressureState = CTRL_PRESSURE_INSPIRATION_MONITORING;
      break;

    case CTRL_PRESSURE_INSPIRATION_MONITORING:
      controlPressureState = CTRL_PRESSURE_EXPIRATION_SETUP;
      break;

    case CTRL_PRESSURE_EXPIRATION_SETUP:
      controlPressureState = CTRL_PRESSURE_EXPIRATION_IDLE;
      break;

    case CTRL_PRESSURE_EXPIRATION_IDLE:
      controlPressureState = CTRL_PRESSURE_INSPIRATION_SETUP;
      break;      

    default:
      controlPressureState = CTRL_PRESSURE_INIT;
      // ToDo: report error
      break;
  }
}

bool CTRL_Timer(uint32_t n)
{
  static uint32_t initialMillis;

  if(n == 0)
  {
	  initialMillis = millis();
  }
  else if((millis() - initialMillis) > n){
	  return true;
  }
    return false;
}