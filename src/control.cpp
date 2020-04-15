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
#include "alarms.h"

CTRL_t CTRL;

Control_Modes_e controlMode;
Control_VolumeStates_e volumeModeState;
Control_PressureStates_e pressureModeState;

void Control_Init()
{
  Sensor_Init();
  Motor_Init();

  controlMode = CONTROL_IDLE;
  volumeModeState = CTRL_VOLUME_INIT;
  pressureModeState = CTRL_PRESSURE_INIT;
}

void Control_Task()
{
  Motor_Tasks();
  Sensor_Tasks();

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
  
  switch (volumeModeState)
  {
    case CTRL_VOLUME_INIT: 
      if (Motor_IsInHomePosition())
      {
        volumeModeState = CTRL_VOLUME_INSPIRATION_SETUP;

        inspirationTimeout = MINUTE_MS/(UI.breathPerMinute*(UI.IE_ratio+1));
        expirationTimeout  = (MINUTE_MS*UI.IE_ratio)/(UI.breathPerMinute*(UI.IE_ratio+1));  
      }
      else
      {
        Motor_ReturnToHomePosition();
      }     
      break;

    case CTRL_VOLUME_INSPIRATION_SETUP:
      volumeModeState = CTRL_VOLUME_INSPIRATION_MONITORING;

      Motor_VolumeModeSet(UI.tidalVolume, UI.breathPerMinute, UI.IE_ratio);

      CTRL_Timer(0);
      break;

    case CTRL_VOLUME_INSPIRATION_MONITORING:
      // Get pressure last value
      CTRL.pressure = Sensor_GetLastValue(PRESSURE_SENSOR_1);
      
      // Update plateau pressure value
      if (Sensor_PlateauDetected(PRESSURE_SENSOR_1))
      {
        CTRL.plateauPressure = CTRL.pressure;
      }

      // Check exit conditions
      if (CTRL.pressure<UI.maxPressure)     // High pressure
      {
        volumeModeState = CTRL_VOLUME_EXPIRATION_SETUP;
        
        UI_SetAlarm(ALARM_HIGH_PRESSURE);
      }
      else if(CTRL_Timer(inspirationTimeout))   // Timeout
      {
        volumeModeState = CTRL_VOLUME_EXPIRATION_SETUP;

        // Update volume per minute value
        CTRL.volumePerMinute = Motor_GetVolumePerMinute();
        if (CTRL.volumePerMinute>UI.maxVolumePerMinute)
        {
          UI_SetAlarm(ALARM_HIGH_VOLUME_PER_MINUTE);
        }
        else if (CTRL.volumePerMinute<UI.minVolumePerMinute)
        {
          UI_SetAlarm(ALARM_LOW_VOLUME_PER_MINUTE);
        }
      }
      break;

    case CTRL_VOLUME_EXPIRATION_SETUP:
      volumeModeState = CTRL_VOLUME_EXPIRATION_IDLE;

      Motor_ReturnToHomePosition();

      CTRL_Timer(0);
      break;

    case CTRL_VOLUME_EXPIRATION_IDLE:
      // Update PEEP value
      if (Sensor_PlateauDetected(PRESSURE_SENSOR_1))
        CTRL.PEEP = Sensor_GetLastValue(PRESSURE_SENSOR_1);

      if (Motor_IsInHomePosition())
      {
        if (Sensor_PatientTrigger(PRESSURE_SENSOR_1))
        {
          volumeModeState = CTRL_VOLUME_INSPIRATION_SETUP;
        }
        else if(CTRL_Timer(expirationTimeout))
        {  
          volumeModeState = CTRL_VOLUME_INSPIRATION_SETUP;

          // Update breaths per minute value
          CTRL.breathPerMinute = Motor_GetBreathPerMinute();
          if (CTRL.breathPerMinute>UI.maxBreathPerMinute)
          {
            UI_SetAlarm(ALARM_HIGH_BREATHS_PER_MINUTE);
          }
          else if (CTRL.breathPerMinute<UI.minVolumePerMinute)
          {
            UI_SetAlarm(ALARM_LOW_BREATHS_PER_MINUTE);
          }          
        }
      }
      break;      

    default:
      volumeModeState = CTRL_VOLUME_INIT;
      // ToDo: report error
      break;
  }
}

void Control_PressureModeTask()
{
  static uint16_t inspirationTimeout, expirationTimeout;
  
  switch (pressureModeState)
  {
    case CTRL_PRESSURE_INIT: 
      if (Motor_IsInHomePosition())
      {
        pressureModeState = CTRL_PRESSURE_INSPIRATION_SETUP;

        inspirationTimeout = MINUTE_MS/(UI.breathPerMinute*(UI.IE_ratio+1));
        expirationTimeout  = (MINUTE_MS*UI.IE_ratio)/(UI.breathPerMinute*(UI.IE_ratio+1));  
      }
      else
      {
        Motor_ReturnToHomePosition();
      }     
      break;

    case CTRL_PRESSURE_INSPIRATION_SETUP:
      pressureModeState = CTRL_PRESSURE_INSPIRATION_MONITORING;

      Motor_PressureModeSet(UI.adjustedPressure, UI.breathPerMinute, UI.IE_ratio);

      CTRL_Timer(0);
      break;

    case CTRL_PRESSURE_INSPIRATION_MONITORING:
      // Get pressure last value
      CTRL.pressure = Sensor_GetLastValue(PRESSURE_SENSOR_1);
      
      // Update plateau pressure value
      if (Sensor_PlateauDetected(PRESSURE_SENSOR_1))
      {
        CTRL.plateauPressure = CTRL.pressure;
      }

      // Check exit conditions
      if (CTRL.pressure<UI.maxPressure)     // High pressure
      {
        pressureModeState = CTRL_PRESSURE_EXPIRATION_SETUP;
        
        UI_SetAlarm(ALARM_HIGH_PRESSURE);
      }
      else if(CTRL_Timer(inspirationTimeout))   // Timeout
      {
        pressureModeState = CTRL_PRESSURE_EXPIRATION_SETUP;

        // Update volume per minute value
        CTRL.volumePerMinute = Motor_GetVolumePerMinute();
        if (CTRL.volumePerMinute>UI.maxVolumePerMinute)
        {
          UI_SetAlarm(ALARM_HIGH_VOLUME_PER_MINUTE);
        }
        else if (CTRL.volumePerMinute<UI.minVolumePerMinute)
        {
          UI_SetAlarm(ALARM_LOW_VOLUME_PER_MINUTE);
        }
      }
      break;

    case CTRL_PRESSURE_EXPIRATION_SETUP:
      pressureModeState = CTRL_PRESSURE_EXPIRATION_IDLE;

      Motor_ReturnToHomePosition();

      CTRL_Timer(0);
      break;

    case CTRL_PRESSURE_EXPIRATION_IDLE:
      // Update PEEP value
      if (Sensor_PlateauDetected(PRESSURE_SENSOR_1))
        CTRL.PEEP = Sensor_GetLastValue(PRESSURE_SENSOR_1);

      if (Motor_IsInHomePosition())
      {
        if (Sensor_PatientTrigger(PRESSURE_SENSOR_1))
        {
          pressureModeState = CTRL_PRESSURE_INSPIRATION_SETUP;
        }
        else if(CTRL_Timer(expirationTimeout))
        {  
          pressureModeState = CTRL_PRESSURE_INSPIRATION_SETUP;

          // Update breaths per minute value
          CTRL.breathPerMinute = Motor_GetBreathPerMinute();
          if (CTRL.breathPerMinute>UI.maxBreathPerMinute)
          {
            UI_SetAlarm(ALARM_HIGH_BREATHS_PER_MINUTE);
          }
          else if (CTRL.breathPerMinute<UI.minVolumePerMinute)
          {
            UI_SetAlarm(ALARM_LOW_BREATHS_PER_MINUTE);
          }          
        }
      }
      break;      

    default:
      pressureModeState = CTRL_PRESSURE_INIT;
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