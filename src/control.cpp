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

Control_Modes_e controlMode;
Control_VentilatorStates_e ventilatorState;

void Control_Init()
{
  Sensor_Init();
  Motor_Init();

  controlMode = CONTROL_IDLE;
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
            ventilatorState = VENTILATOR_SETUP;
            break;

          case UI_PRESSURE_CONTROL:
            controlMode = PRESSURE_CONTROL;
            ventilatorState = VENTILATOR_SETUP;
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
          ventilatorState = VENTILATOR_SETUP;
          break; 
        }         
      }

      Control_VolumeModeTask();
      break;

    case PRESSURE_CONTROL:
      if (UI.setUpComplete)
      {
        UI.setUpComplete = false;

        if (UI.selectedMode==UI_VOLUME_CONTROL)
        {
          controlMode = VOLUME_CONTROL;
          ventilatorState = VENTILATOR_SETUP;
          break; 
        }         
      }

      Control_PressureModeTask();
      break;      
  
    default:
      // ToDo: report error
      break;
  }
}

void Control_VolumeModeTask()
{
  switch (ventilatorState)
  {
    case VENTILATOR_SETUP:
      ventilatorState = VENTILATOR_INSPIRATION;
      break;

    case VENTILATOR_INSPIRATION:
      ventilatorState = VENTILATOR_EXPIRATION;
      break;

    case VENTILATOR_EXPIRATION:
      ventilatorState = VENTILATOR_INSPIRATION;
      break;

    default:
      ventilatorState = VENTILATOR_SETUP;
      // ToDo: report error
      break;
  }
}

void Control_PressureModeTask()
{
  switch (ventilatorState)
  {
    case VENTILATOR_SETUP:
      ventilatorState = VENTILATOR_INSPIRATION;
      break;

    case VENTILATOR_INSPIRATION:
      ventilatorState = VENTILATOR_EXPIRATION;
      break;

    case VENTILATOR_EXPIRATION:
      ventilatorState = VENTILATOR_INSPIRATION;
      break;

    default:
      // ToDo: report error
      break;
  }
}