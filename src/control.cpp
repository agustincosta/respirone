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

  /*
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
            volumeModeState = CTRL_VOLUME_INIT;
            break;

          case UI_PRESSURE_CONTROL:
            controlMode = PRESSURE_CONTROL;
            pressureModeState = CTRL_PRESSURE_INIT;
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
          pressureModeState = CTRL_PRESSURE_INIT;
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
          volumeModeState = CTRL_VOLUME_INIT;
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
  }*/
}

void Control_VolumeModeTask()
{
  /*
  static uint16_t inspirationTimeout, expirationTimeout;
  
  switch (volumeModeState)
  {
    case CTRL_VOLUME_INIT: 
      if (Motor_IsInHomePosition())
      {
        volumeModeState = CTRL_VOLUME_INSPIRATION_SETUP;

        inspirationTimeout = MINUTE_MS/(UI.breathsMinute*(UI.i_e+1));
        expirationTimeout  = (MINUTE_MS*UI.i_e)/(UI.breathsMinute*(UI.i_e+1));  
      }
      else
      {
        Motor_ReturnToHomePosition();
      }     
      break;

    case CTRL_VOLUME_INSPIRATION_SETUP:
      volumeModeState = CTRL_VOLUME_INSPIRATION_MONITORING;

      Motor_VolumeModeSet(UI.tidalVolume, UI.breathsMinute, UI.i_e);

      CTRL_Timer(0);
      break;

    case CTRL_VOLUME_INSPIRATION_MONITORING:
      // Update pressure last value
      CTRL.pressure = Sensor_GetLastValue(PRESSURE_SENSOR_1);
      
      // Check exit conditions
      if (CTRL.pressure<UI.maxPressure)     // High pressure
      {
        volumeModeState = CTRL_VOLUME_EXPIRATION_SETUP;
        
        UI_SetAlarm(ALARM_HIGH_PRESSURE);
      }
      else if(CTRL_Timer(inspirationTimeout))   // Timeout
      {
        volumeModeState = CTRL_VOLUME_EXPIRATION_SETUP;

        // Update peak pressure value
        CTRL.peakPressure = Sensor_GetPeakValue(PRESSURE_SENSOR_1);

        // Update plateau pressure value
        if (Sensor_PlateauDetected(PRESSURE_SENSOR_1))
        {
          CTRL.plateauPressure = Sensor_GetPlateauValue(PRESSURE_SENSOR_1);
        }

        // Update volume per minute value
        // CTRL.volumeMinute = Motor_GetVolumePerMinute();
        // if (CTRL.volumeMinute>UI.maxVolumeMinute)
        // {
          // volumeModeState = CTRL_VOLUME_EXPIRATION_SETUP;
          // UI_SetAlarm(ALARM_HIGH_VOLUME_PER_MINUTE);
        // }
        // else if (CTRL.volumeMinute<UI.minVolumeMinute)
        // {
          ////volumeModeState = CTRL_VOLUME_EXPIRATION_SETUP;
          // UI_SetAlarm(ALARM_LOW_VOLUME_PER_MINUTE);
        // }
      }
      break;

    case CTRL_VOLUME_EXPIRATION_SETUP:
      volumeModeState = CTRL_VOLUME_EXPIRATION_IDLE;

      Motor_ReturnToHomePosition();

      CTRL_Timer(0);
      break;

    case CTRL_VOLUME_EXPIRATION_IDLE:
      // Get pressure last value
      CTRL.pressure = Sensor_GetLastValue(PRESSURE_SENSOR_1);

      if (CTRL.pressure<UI.TrP) // Trigger from patient
      {
        if (Motor_IsInHomePosition())
        {
          volumeModeState = CTRL_VOLUME_INSPIRATION_SETUP;
        }
        else
        {
          //ToDo> report error
        }
      }
      else if(CTRL_Timer(expirationTimeout))
      {  
        if (Motor_IsInHomePosition())
        {
          volumeModeState = CTRL_VOLUME_INSPIRATION_SETUP;
		  
		  // Update PEEP value
		  if (Sensor_PlateauDetected(PRESSURE_SENSOR_1))
			CTRL.PEEP = Sensor_GetPlateauValue(PRESSURE_SENSOR_1);

          // Update breaths per minute value
          CTRL.breathsMinute = Motor_GetBreathsPerMinute();
          if (CTRL.breathsMinute>UI.maxBreathsMinute)
          {
            UI_SetAlarm(ALARM_HIGH_BREATHS_PER_MINUTE);
          }
          else if (CTRL.breathsMinute<UI.minVolumeMinute)
          {
            UI_SetAlarm(ALARM_LOW_BREATHS_PER_MINUTE);
          }      
        }
        else
        {
          //ToDo> report error
        }
            
      }
      break;      

    default:
      volumeModeState = CTRL_VOLUME_INIT;
      // ToDo: report error
      break;
  }
  */
}

void Control_PressureModeTask()
{
  /*
  static uint16_t inspirationTimeout, expirationTimeout;
  
  switch (pressureModeState)
  {
    case CTRL_PRESSURE_INIT: 
      if (Motor_IsInHomePosition())
      {
        pressureModeState = CTRL_PRESSURE_INSPIRATION_SETUP;

        inspirationTimeout = MINUTE_MS/(UI.breathsMinute*(UI.i_e+1));
        expirationTimeout  = (MINUTE_MS*UI.i_e)/(UI.breathsMinute*(UI.i_e+1));  
      }
      else
      {
        Motor_ReturnToHomePosition();
      }     
      break;

    case CTRL_PRESSURE_INSPIRATION_SETUP:
      pressureModeState = CTRL_PRESSURE_INSPIRATION_MONITORING;

      Motor_PressureModeSet(UI.adjustedPressure, UI.breathsMinute, UI.i_e);

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
        CTRL.volumeMinute = Motor_GetVolumePerMinute();
        if (CTRL.volumeMinute>UI.maxVolumeMinute)
        {
          UI_SetAlarm(ALARM_HIGH_VOLUME_PER_MINUTE);
        }
        else if (CTRL.volumeMinute<UI.minVolumeMinute)
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
      // Get pressure last value
      CTRL.pressure = Sensor_GetLastValue(PRESSURE_SENSOR_1);

      if (CTRL.pressure<UI.TrP) // Trigger from patient
      {
		  if (Motor_IsInHomePosition())
        {
          pressureModeState = CTRL_PRESSURE_INSPIRATION_SETUP;
        }
        else
        {
          //ToDo> report error
        }		
      }
      else if(CTRL_Timer(expirationTimeout))
      {
        if (Motor_IsInHomePosition())
        {
          pressureModeState = CTRL_PRESSURE_INSPIRATION_SETUP;

          // Update breaths per minute value
          CTRL.breathsMinute = Motor_GetBreathsPerMinute();
          if (CTRL.breathsMinute>UI.maxBreathsMinute)
          {
            UI_SetAlarm(ALARM_HIGH_BREATHS_PER_MINUTE);
          }
          else if (CTRL.breathsMinute<UI.minVolumeMinute)
          {
            UI_SetAlarm(ALARM_LOW_BREATHS_PER_MINUTE);
          }
        }
        else
        {
          //ToDo> report error
        }    		
      }          
      break;      

    default:
      pressureModeState = CTRL_PRESSURE_INIT;
      // ToDo: report error
      break;
  }
  */
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