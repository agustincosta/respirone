#include "user_interface.h"
#include "control.h"
#include "alarms.h"
#include "config.h"
#include "time.h"
#include <LiquidCrystal.h>

//LCD
LiquidCrystal lcd(DISPLAY_RS_PIN, DISPLAY_ENABLE_PIN, DISPLAY_D0_PIN, DISPLAY_D1_PIN, DISPLAY_D2_PIN, DISPLAY_D3_PIN);

// UI Variables
UI_t tempParam;
UI_t UI;

UI_states_e               uiTask;
UI_SetParametersStates_e  uiState;
UI_ShowParametersStates_e spState;
UI_AlarmMedicalStates_e   alarmMedicalState;
UI_AlarmSystemStates_e    alarmSystemState;

// Control
CTRL_t showParam;

// Alarm
ALARM_t ALARM;
uint8_t actualAlarm; 

// Button Debounce 
DebounceStates_t  debounceState[ARDUINO_PIN_QTY];
bool              buttonState[ARDUINO_PIN_QTY];

// User Interface
void UI_Init()
{
  // Alarm
  pinMode(LED_MEDICAL_ALARM_PIN, OUTPUT);
  pinMode(BUZZER_ALARM_PIN, OUTPUT);

  ALARM.enable = true;

  if(!tempParam.initBeepOff)
    tone(BUZZER_ALARM_PIN, 500, 500);
  
  // Init FSM
  if(tempParam.notFirstInit)
  {
    uiTask = UI_SHOW_VERSION;
  }
  else 
  {
    tempParam.notFirstInit = true;
    uiTask = UI_DELAY_INIT;
  }
  
  uiState = UI_SET_MODE_AUTO;

  // Button
  pinMode(BUTTON_UP_PIN,    INPUT_PULLUP); 
  pinMode(BUTTON_DOWN_PIN,  INPUT_PULLUP); 
  pinMode(BUTTON_MENU_PIN,  INPUT_PULLUP); 
  pinMode(BUTTON_ENTER_PIN, INPUT_PULLUP); 
  pinMode(BUTTON_BACK_PIN,  INPUT_PULLUP); 
  pinMode(BUTTON_CIRCLE_PIN,  INPUT_PULLUP);

  // Timer
  UI_Timer(0); 

  // Default parameters
  UI_LoadParam(1);
}

void UI_Task()
{
  char stringAux[DISPLAY_COLUMNS+1];
  static bool initialSetUpDone;
  stringAux[0] = 0;
  
  switch (uiTask)
  {
    case UI_DELAY_INIT:
      if(UI_Timer(TIMEOUT_DELAY_INIT))
      {
        UI_Timer(0);
        uiTask = UI_SHOW_VERSION;

        // init Display
        lcd.begin(DISPLAY_COLUMNS, DISPLAY_ROWS);   // initialize the lcd 
        lcd.home ();                                // go home
        UI_DisplayClear();

        strcat(stringAux, FIRMWARE_VERSION);
        strcat(stringAux, " - ");
        strcat(stringAux, SERIAL_NUMBER_STR);

        UI_DisplayMessage(0,0,DISPLAY_PROJECT_NAME);
        UI_DisplayMessage(0,1,stringAux);        

      }
      break;

    case UI_SHOW_VERSION:
      if(UI_Timer(TIMEOUT_SHOW_VERSION) || (!tempParam.notFirstInit))
      {
        UI_DisplayClear();
        UI_DisplayMessage(0,0,DISPLAY_PROJECT_NAME);
        UI_DisplayMessage(0,1,DISPLAY_LAUNCH_MENU);  
        UI_Timer(0);
        uiTask = UI_WAITING_BUTTON;
      }

      break;

    case UI_WAITING_BUTTON:
      if(UI_ButtonDebounce(BUTTON_MENU_PIN))
      {
        UI_DisplayClear();
        UI_DisplayMessage(0,0,DISPLAY_SELECT_MODE);
        UI_DisplayMessage(0,1,DISPLAY_AUTO_MODE);
        uiTask = UI_SET_UP_PAREMETERS;
        UI_Timer(0);
      }
      break;

    case UI_SET_UP_PAREMETERS:

      UI_SetParametersTask();

      if(UI.setUpComplete)
      {
        uiTask = UI_SHOW_PARAMETERS;
        initialSetUpDone = true;
        UI.ventilationOn = true;
        UI_Timer(0);
      }
      if(initialSetUpDone) 
      {
        if(UI_ButtonDebounce(BUTTON_CIRCLE_PIN))
        {
          uiTask = UI_CANCEL_EDITION;
          UI_Timer(0);
        }
        else if(UI_ButtonDebounce(BUTTON_MENU_PIN))
        {
          uiTask = UI_RESTART_CONFIG;
          UI_Timer(0);
        }
      }
      break;

    case UI_SHOW_PARAMETERS:
      if (UI_ActiveMedicalAlarms()||UI_ActiveSystemAlarms())
      {
        uiTask = UI_ALARMS_MANAGEMENT;
        digitalWrite(LED_MEDICAL_ALARM_PIN, HIGH);
      }
      else if(UI_ButtonDebounce(BUTTON_BACK_PIN))
      {
        if(tempParam.selectedMode == UI_PRESSURE_CONTROL)
          {
            UI_DisplayClear();
            UI_DisplayMessage(0,0,DISPLAY_ADJUSTED_PRESSURE);

            itoa(tempParam.adjustedPressure,stringAux,10);
            strcat(stringAux, "cm.H2O");
            UI_DisplayMessage(0,1,stringAux);

            uiState = UI_SET_ADJUSTED_PRESSURE;
            UI_Timer(0);
          }
          else if(tempParam.selectedMode == UI_VOLUME_CONTROL)
          {
            UI_DisplayClear();
            UI_DisplayMessage(0,0,DISPLAY_TIDAL_VOLUME);

            itoa(tempParam.tidalVolume,stringAux,10);
            strcat(stringAux, "ml");
            UI_DisplayMessage(0,1,stringAux);

            uiState = UI_SET_TIDAL_VOLUME;
            UI_Timer(0);
          }
          else if(tempParam.selectedMode == UI_AUTOMATIC_CONTROL)
          {
            UI_DisplayClear();
            UI_DisplayMessage(0,0,DISPLAY_SELECT_MODE);
            UI_DisplayMessage(0,1,DISPLAY_AUTO_MODE);
            uiState = UI_SET_MODE_AUTO;
            UI_Timer(0);
          }

          uiTask = UI_SET_UP_PAREMETERS;

      } 
      else if(UI_ButtonDebounce(BUTTON_CIRCLE_PIN))
      {
        UI_DisplayClear();
        UI_DisplayMessage(0,0,DISPLAY_STOP_VENTILATION);
        UI_DisplayMessage(0,1,DISPLAY_CONFIRMATION_OPTIONS);
        uiTask = UI_STOP_VENTILATION_CONFIRMATION;
      }
      else
      {
        UI_ShowParametersTask();
      }
      break;

    case UI_ALARMS_MANAGEMENT:

      if (UI_ActiveSystemAlarms())
      {
        UI_SystemAlarmTask();
      }
      else if (UI_ActiveMedicalAlarms())
      {
        UI_MedicalAlarmTask();
      }
      else
      {
        uiTask = UI_SHOW_PARAMETERS;
      }
      break;

    case UI_STOP_VENTILATION_CONFIRMATION:
      if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        UI.ventilationOn = false;
        UI.stopVentilation = true;
        tempParam.initBeepOff = true;
        UI_DisplayClear();
        UI_Init();
      }
      else if((UI_ButtonDebounce(BUTTON_BACK_PIN)) || (UI_Timer(TIMEOUT_STOP_VENTILATION_CONFIRM)))
      {
        UI_DisplayClear();
        uiTask = UI_SHOW_PARAMETERS;
        UI_Timer(0);
      }
      break;

    case UI_CANCEL_EDITION:
      UI_ButtonDebounce(BUTTON_CIRCLE_PIN);
      if((buttonState[BUTTON_CIRCLE_PIN]) && (UI_Timer(TIMEOUT_CANCEL_EDITION)))
      {
        UI_DisplayClear();
        uiTask = UI_SHOW_PARAMETERS;
        UI_Timer(0);        
      }
      else if(!(buttonState[BUTTON_CIRCLE_PIN]))
      {
        uiTask = UI_SET_UP_PAREMETERS;
        UI_Timer(0);
      }
      break;

    case UI_RESTART_CONFIG:
      UI_ButtonDebounce(BUTTON_MENU_PIN);
      if((buttonState[BUTTON_MENU_PIN]) && (UI_Timer(TIMEOUT_RESTART_CONFIG)))
      {
        if(tempParam.selectedMode == UI_PRESSURE_CONTROL)
          {
            UI_DisplayClear();
            UI_DisplayMessage(0,0,DISPLAY_ADJUSTED_PRESSURE);

            itoa(tempParam.adjustedPressure,stringAux,10);
            strcat(stringAux, "cm.H2O");
            UI_DisplayMessage(0,1,stringAux);

            uiState = UI_SET_ADJUSTED_PRESSURE;
            UI_Timer(0);
          }
          else if(tempParam.selectedMode == UI_VOLUME_CONTROL)
          {
            UI_DisplayClear();
            UI_DisplayMessage(0,0,DISPLAY_TIDAL_VOLUME);

            itoa(tempParam.tidalVolume,stringAux,10);
            strcat(stringAux, "ml");
            UI_DisplayMessage(0,1,stringAux);

            uiState = UI_SET_TIDAL_VOLUME;
            UI_Timer(0);
          }
          else if(tempParam.selectedMode == UI_AUTOMATIC_CONTROL)
          {
            UI_DisplayClear();
            UI_DisplayMessage(0,0,DISPLAY_SELECT_MODE);
            UI_DisplayMessage(0,1,DISPLAY_AUTO_MODE);
            uiState = UI_SET_MODE_AUTO;
            UI_Timer(0);
          }

          uiTask = UI_SET_UP_PAREMETERS;        
      }
      else if(!(buttonState[BUTTON_MENU_PIN]))
      {
        uiTask = UI_SET_UP_PAREMETERS;
        UI_Timer(0);
      }      
      break;

  default:
    uiTask = UI_WAITING_BUTTON;
    break;
  }

}

void UI_SetParametersTask()
{
  char stringAux[16];

  stringAux[0] = 0;
  
  switch (uiState) 
  {
////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////// MODE ///////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
    case UI_SET_MODE_AUTO:
        if(UI_ButtonDebounce(BUTTON_UP_PIN))  //pre
        {
          #if PRESSURE_MODE
            UI_DisplayMessage(0,1,DISPLAY_PRESSURE_MODE);
            uiState = UI_SET_MODE_PRESSURE;
          #else
            UI_DisplayMessage(0,1,DISPLAY_VOLUME_MODE);
            uiState = UI_SET_MODE_VOLUME;
          #endif
        } 
        else if(UI_ButtonDebounce(BUTTON_DOWN_PIN)) //vol
        {
          UI_DisplayMessage(0,1,DISPLAY_VOLUME_MODE);
          uiState = UI_SET_MODE_VOLUME;
        }
        else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
        {
          tempParam.selectedMode = UI_AUTOMATIC_CONTROL;
          uiState = UI_SET_DEFAULT_PARAMETERS;
          UI_Timer(0);
        }
        else if(UI_Timer(TIMEOUT_BLINK))
        {
          uiState = UI_BLINK_AUTO;
          UI_DisplayMessage(0,1,DISPLAY_EMPTY_LINE);
          UI_Timer(0);
        }
        break;

    case UI_BLINK_AUTO:
      if(UI_ButtonDebounce(BUTTON_UP_PIN))  //pre
      {
        #if PRESSURE_MODE
          UI_DisplayMessage(0,1,DISPLAY_PRESSURE_MODE);
          uiState = UI_SET_MODE_PRESSURE;
        #else
          UI_DisplayMessage(0,1,DISPLAY_VOLUME_MODE);
          uiState = UI_SET_MODE_VOLUME;
        #endif
      } 
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN)) //vol
      {
        UI_DisplayMessage(0,1,DISPLAY_VOLUME_MODE);
        uiState = UI_SET_MODE_VOLUME;
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        tempParam.selectedMode = UI_AUTOMATIC_CONTROL;
        uiState = UI_SET_DEFAULT_PARAMETERS;
        UI_Timer(0);
      }
      else if(UI_Timer(TIMEOUT_BLINK/2))
      {
        UI_DisplayMessage(0,1,DISPLAY_AUTO_MODE);
        uiState = UI_SET_MODE_AUTO; 
        UI_Timer(0);
      }
      break;

    case UI_SET_MODE_VOLUME:
      if(UI_ButtonDebounce(BUTTON_UP_PIN))  //auto
      {
        UI_DisplayMessage(0,1,DISPLAY_AUTO_MODE);
        uiState = UI_SET_MODE_AUTO;
      } 
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN)) //pre
      {
        #if PRESSURE_MODE
          UI_DisplayMessage(0,1,DISPLAY_PRESSURE_MODE);
          uiState = UI_SET_MODE_PRESSURE;
        #else
          UI_DisplayMessage(0,1,DISPLAY_AUTO_MODE);
          uiState = UI_SET_MODE_AUTO;
        #endif
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        UI_DisplayMessage(0,1,DISPLAY_VOLUME_MODE);
        tempParam.selectedMode = UI_VOLUME_CONTROL;
        uiState = UI_DELAY_END_MODE;
        UI_Timer(0);
      }
      else if(UI_Timer(TIMEOUT_BLINK))
      {
        uiState = UI_BLINK_VOLUME;
        UI_DisplayMessage(0,1,DISPLAY_EMPTY_LINE);
        UI_Timer(0);
      }
      break;

    case UI_BLINK_VOLUME:
      if(UI_ButtonDebounce(BUTTON_UP_PIN))  //auto
      {
        UI_DisplayMessage(0,1,DISPLAY_AUTO_MODE);
        uiState = UI_SET_MODE_AUTO;
      } 
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN)) //pre
      {
        #if PRESSURE_MODE
          UI_DisplayMessage(0,1,DISPLAY_PRESSURE_MODE);
          uiState = UI_SET_MODE_PRESSURE;
        #else
          UI_DisplayMessage(0,1,DISPLAY_AUTO_MODE);
          uiState = UI_SET_MODE_AUTO;
        #endif
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        UI_DisplayMessage(0,1,DISPLAY_VOLUME_MODE);
        tempParam.selectedMode = UI_VOLUME_CONTROL;
        uiState = UI_DELAY_END_MODE;
        UI_Timer(0);
      }
      else if(UI_Timer(TIMEOUT_BLINK/2))
      {
        uiState = UI_SET_MODE_VOLUME;
        UI_DisplayMessage(0,1,DISPLAY_VOLUME_MODE);
        UI_Timer(0);
      }
      break;

    case UI_SET_MODE_PRESSURE:
      if(UI_ButtonDebounce(BUTTON_UP_PIN))  //vol
      {
        UI_DisplayMessage(0,1,DISPLAY_VOLUME_MODE);
        uiState = UI_SET_MODE_VOLUME;
      } 
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN)) //auto
      {
        UI_DisplayMessage(0,1,DISPLAY_AUTO_MODE);
        uiState = UI_SET_MODE_AUTO;
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        UI_DisplayMessage(0,1,DISPLAY_PRESSURE_MODE);
        tempParam.selectedMode = UI_PRESSURE_CONTROL;
        uiState = UI_DELAY_END_MODE;
        UI_Timer(0);
      }
      else if(UI_Timer(TIMEOUT_BLINK))
      {
        uiState = UI_BLINK_PRESSURE;
        UI_DisplayMessage(0,1,DISPLAY_EMPTY_LINE);
        UI_Timer(0);
      }
      break;

    case UI_BLINK_PRESSURE:
      if(UI_ButtonDebounce(BUTTON_UP_PIN))  //vol
      {
        UI_DisplayMessage(0,1,DISPLAY_VOLUME_MODE);
        uiState = UI_SET_MODE_VOLUME;
      } 
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN)) //auto
      {
        UI_DisplayMessage(0,1,DISPLAY_AUTO_MODE);
        uiState = UI_SET_MODE_AUTO;
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        UI_DisplayMessage(0,1,DISPLAY_PRESSURE_MODE);
        tempParam.selectedMode = UI_PRESSURE_CONTROL;
        uiState = UI_DELAY_END_MODE;
        UI_Timer(0);
      }
      else if(UI_Timer(TIMEOUT_BLINK/2))
      {
        uiState = UI_SET_MODE_PRESSURE;
        UI_DisplayMessage(0,1,DISPLAY_PRESSURE_MODE);
        UI_Timer(0);
      }
      break;

    case UI_DELAY_END_MODE:
      if(UI_Timer(TIMEOUT_SHOW_SELECTED_PARAM))
      {
        if(tempParam.selectedMode == UI_PRESSURE_CONTROL)
        {
          UI_DisplayClear();
          UI_DisplayMessage(0,0,DISPLAY_ADJUSTED_PRESSURE);

          itoa(tempParam.adjustedPressure,stringAux,10);
          strcat(stringAux, "cm.H2O");
          UI_DisplayMessage(0,1,stringAux);

          uiState = UI_SET_ADJUSTED_PRESSURE;
          UI_Timer(0);
        }
        else if(tempParam.selectedMode == UI_VOLUME_CONTROL)
        {
          UI_DisplayClear();
          UI_DisplayMessage(0,0,DISPLAY_TIDAL_VOLUME);

          itoa(tempParam.tidalVolume,stringAux,10);
          strcat(stringAux, "ml");
          UI_DisplayMessage(0,1,stringAux);

          uiState = UI_SET_TIDAL_VOLUME;
          UI_Timer(0);
        }
      }
      break;
////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////// ADJUSTED PRESSURE //////////
////////////////////////////////////////////////////////////////////////////////////////////////////////      

    case UI_SET_ADJUSTED_PRESSURE:
      if(UI_ButtonDebounce(BUTTON_UP_PIN))  
      {
        (tempParam.adjustedPressure < MAX_ADJUSTED_PRESSURE) ? tempParam.adjustedPressure += ADJUSTED_PRESSURE_STEP : tempParam.adjustedPressure = MAX_ADJUSTED_PRESSURE;
        itoa(tempParam.adjustedPressure,stringAux,10);
        strcat(stringAux, "cm.H2O");
        UI_DisplayMessage(0,1,stringAux);

      } 
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN)) 
      {
        (tempParam.adjustedPressure > MIN_ADJUSTED_PRESSURE)? tempParam.adjustedPressure -= ADJUSTED_PRESSURE_STEP : tempParam.adjustedPressure = MIN_ADJUSTED_PRESSURE;
        itoa(tempParam.adjustedPressure,stringAux,10);
        strcat(stringAux, "cm.H2O");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        itoa(tempParam.adjustedPressure,stringAux,10);
        strcat(stringAux, "cm.H2O");
        UI_DisplayMessage(0,1,stringAux);
        //UI.adjustedPressure = tempParam.adjustedPressure;
        uiState = UI_DELAY_END_ADJUSTED_PRESSURE;
        UI_Timer(0);
      }
      else if(UI_ButtonDebounce(BUTTON_BACK_PIN))
      {
        UI_DisplayClear();
        UI_DisplayMessage(0,0,DISPLAY_SELECT_MODE);
        UI_DisplayMessage(0,1,DISPLAY_AUTO_MODE);
        uiState = UI_SET_MODE_AUTO;
        UI_Timer(0);       
      }      
      else if(UI_Timer(TIMEOUT_BLINK))
      {
        uiState = UI_BLINK_ADJUSTED_PRESSURE;
        UI_DisplayMessage(0,1,DISPLAY_EMPTY_LINE);
        UI_Timer(0);
      }    
      break;

    case UI_BLINK_ADJUSTED_PRESSURE:
      if(UI_ButtonDebounce(BUTTON_UP_PIN))  
      {
        (tempParam.adjustedPressure < MAX_ADJUSTED_PRESSURE) ? tempParam.adjustedPressure += ADJUSTED_PRESSURE_STEP : tempParam.adjustedPressure = MAX_ADJUSTED_PRESSURE;
        itoa(tempParam.adjustedPressure,stringAux,10);
        strcat(stringAux, "cm.H2O");
        UI_DisplayMessage(0,1,stringAux);
      } 
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN)) 
      {
        (tempParam.adjustedPressure > MIN_ADJUSTED_PRESSURE) ? tempParam.adjustedPressure -= ADJUSTED_PRESSURE_STEP : tempParam.adjustedPressure = MIN_ADJUSTED_PRESSURE;
        itoa(tempParam.adjustedPressure,stringAux,10);
        strcat(stringAux, "cm.H2O");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        itoa(tempParam.adjustedPressure,stringAux,10);
        strcat(stringAux, "cm.H2O");
        UI_DisplayMessage(0,1,stringAux);
        //UI.adjustedPressure = tempParam.adjustedPressure;
        uiState = UI_DELAY_END_ADJUSTED_PRESSURE;
        UI_Timer(0);
      }
      else if(UI_ButtonDebounce(BUTTON_BACK_PIN))
      {
        UI_DisplayClear();
        UI_DisplayMessage(0,0,DISPLAY_SELECT_MODE);
        UI_DisplayMessage(0,1,DISPLAY_AUTO_MODE);
        uiState = UI_SET_MODE_AUTO;
        UI_Timer(0);       
      }      
      else if(UI_Timer(TIMEOUT_BLINK/2))
      {
        itoa(tempParam.adjustedPressure,stringAux,10);
        strcat(stringAux, "cm.H2O");
        UI_DisplayMessage(0,1,stringAux);
        uiState = UI_SET_ADJUSTED_PRESSURE;
        UI_Timer(0);
      }        

      break;

    case UI_DELAY_END_ADJUSTED_PRESSURE:
      if(UI_Timer(TIMEOUT_SHOW_SELECTED_PARAM))
      {
          UI_DisplayClear();
          UI_DisplayMessage(0,0,DISPLAY_VOLUME_MINUTE_M);

          itoa(tempParam.maxVolumeMinute,stringAux,10);
          strcat(stringAux, "L/m");
          UI_DisplayMessage(0,1,stringAux);

          uiState = UI_SET_VOLUME_MINUTE_M;
          UI_Timer(0);
      }
      break;

////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////// TIDAL VOLUME ///////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////  

    case UI_SET_TIDAL_VOLUME:
      if(UI_ButtonDebounce(BUTTON_UP_PIN))  
      {
        (tempParam.tidalVolume < MAX_ADJUSTED_VOLUME) ? tempParam.tidalVolume += ADJUSTED_VOLUME_STEP : tempParam.tidalVolume = MAX_ADJUSTED_VOLUME;
        itoa(tempParam.tidalVolume,stringAux,10);
        strcat(stringAux, "ml");
        UI_DisplayMessage(0,1,stringAux);

      } 
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN)) 
      {
        (tempParam.tidalVolume > MIN_ADJUSTED_VOLUME)? tempParam.tidalVolume -= ADJUSTED_VOLUME_STEP : tempParam.tidalVolume = MIN_ADJUSTED_VOLUME;
        itoa(tempParam.tidalVolume,stringAux,10);
        strcat(stringAux, "ml");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        itoa(tempParam.tidalVolume,stringAux,10);
        strcat(stringAux, "ml");
        UI_DisplayMessage(0,1,stringAux);
        //UI.tidalVolume = tempParam.tidalVolume;
        uiState = UI_DELAY_END_TIDAL_VOLUME;
        UI_Timer(0);
      }
      else if(UI_ButtonDebounce(BUTTON_BACK_PIN))
      {
        UI_DisplayClear();
        UI_DisplayMessage(0,0,DISPLAY_SELECT_MODE);
        UI_DisplayMessage(0,1,DISPLAY_AUTO_MODE);
        uiState = UI_SET_MODE_AUTO;
        UI_Timer(0);       
      }      
      else if(UI_Timer(TIMEOUT_BLINK))
      {
        uiState = UI_BLINK_TIDAL_VOLUME;
        UI_DisplayMessage(0,1,DISPLAY_EMPTY_LINE);
        UI_Timer(0);
      }    
      break;

    case UI_BLINK_TIDAL_VOLUME:
      if(UI_ButtonDebounce(BUTTON_UP_PIN))  
      {
        (tempParam.tidalVolume < MAX_ADJUSTED_VOLUME) ? tempParam.tidalVolume += ADJUSTED_VOLUME_STEP : tempParam.tidalVolume = MAX_ADJUSTED_VOLUME;
        itoa(tempParam.tidalVolume,stringAux,10);
        strcat(stringAux, "ml");
        UI_DisplayMessage(0,1,stringAux);
      } 
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN)) 
      {
        (tempParam.tidalVolume > MIN_ADJUSTED_VOLUME) ? tempParam.tidalVolume -= ADJUSTED_VOLUME_STEP : tempParam.tidalVolume = MIN_ADJUSTED_VOLUME;
        itoa(tempParam.tidalVolume,stringAux,10);
        strcat(stringAux, "ml");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        itoa(tempParam.tidalVolume,stringAux,10);
        strcat(stringAux, "ml");
        UI_DisplayMessage(0,1,stringAux);
        //UI.tidalVolume = tempParam.tidalVolume;
        uiState = UI_DELAY_END_TIDAL_VOLUME;
        UI_Timer(0);
      }
      else if(UI_ButtonDebounce(BUTTON_BACK_PIN))
      {
        UI_DisplayClear();
        UI_DisplayMessage(0,0,DISPLAY_SELECT_MODE);
        UI_DisplayMessage(0,1,DISPLAY_AUTO_MODE);
        uiState = UI_SET_MODE_AUTO;
        UI_Timer(0);       
      }      
      else if(UI_Timer(TIMEOUT_BLINK/2))
      {
        itoa(tempParam.tidalVolume,stringAux,10);
        strcat(stringAux, "ml");
        UI_DisplayMessage(0,1,stringAux);
        uiState = UI_SET_TIDAL_VOLUME;
        UI_Timer(0);
      }        

      break;

    case UI_DELAY_END_TIDAL_VOLUME:
      if(UI_Timer(TIMEOUT_SHOW_SELECTED_PARAM))
      {
          UI_DisplayClear();
          UI_DisplayMessage(0,0,DISPLAY_VOLUME_MINUTE_M);

          itoa(tempParam.maxVolumeMinute,stringAux,10);
          strcat(stringAux, "L/m");
          UI_DisplayMessage(0,1,stringAux);

          uiState = UI_SET_VOLUME_MINUTE_M;
          UI_Timer(0);
      }
      break;

////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////// MAX VOLUME MINUTE //////////
////////////////////////////////////////////////////////////////////////////////////////////////////////        

    case UI_SET_VOLUME_MINUTE_M:
      if(UI_ButtonDebounce(BUTTON_UP_PIN)) 
      {
        (tempParam.maxVolumeMinute < MAX_ADJUSTED_MAX_VOLUME_MINUTE) ? tempParam.maxVolumeMinute += ADJUSTED_MAX_VOLUME_MINTUE_STEP : tempParam.maxVolumeMinute = MAX_ADJUSTED_MAX_VOLUME_MINUTE;
        itoa(tempParam.maxVolumeMinute,stringAux,10);
        strcat(stringAux, "L/m");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN))
      {
        (tempParam.maxVolumeMinute > MIN_ADJUSTED_MAX_VOLUME_MINUTE)? tempParam.maxVolumeMinute -= ADJUSTED_MAX_VOLUME_MINTUE_STEP : tempParam.maxVolumeMinute = MIN_ADJUSTED_MAX_VOLUME_MINUTE;
        itoa(tempParam.maxVolumeMinute,stringAux,10);
        strcat(stringAux, "L/m");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        itoa(tempParam.maxVolumeMinute,stringAux,10);
        strcat(stringAux, "L/m");
        UI_DisplayMessage(0,1,stringAux);
        //UI.maxVolumeMinute = tempParam.maxVolumeMinute;
        uiState = UI_DELAY_END_VOLUME_MINUTE_M;
        UI_Timer(0);
      }
      else if(UI_ButtonDebounce(BUTTON_BACK_PIN))
      {
        if(tempParam.selectedMode == UI_PRESSURE_CONTROL)
        {
          UI_DisplayClear();
          UI_DisplayMessage(0,0,DISPLAY_ADJUSTED_PRESSURE);

          itoa(tempParam.adjustedPressure,stringAux,10);
          strcat(stringAux, "cm.H2O");
          UI_DisplayMessage(0,1,stringAux);

          uiState = UI_SET_ADJUSTED_PRESSURE;
          UI_Timer(0);
        }
        else if(tempParam.selectedMode == UI_VOLUME_CONTROL)
        {
          UI_DisplayClear();
          UI_DisplayMessage(0,0,DISPLAY_TIDAL_VOLUME);

          itoa(tempParam.tidalVolume,stringAux,10);
          strcat(stringAux, "ml");
          UI_DisplayMessage(0,1,stringAux);

          uiState = UI_SET_TIDAL_VOLUME;
          UI_Timer(0);
        }       
      }      
      else if(UI_Timer(TIMEOUT_BLINK))
      {
        uiState = UI_BLINK_VOLUME_MINUTE_M;
        UI_DisplayMessage(0,1,DISPLAY_EMPTY_LINE);
        UI_Timer(0);
      }        
      break;

    case UI_BLINK_VOLUME_MINUTE_M:
      if(UI_ButtonDebounce(BUTTON_UP_PIN))  
      {
        (tempParam.maxVolumeMinute < MAX_ADJUSTED_MAX_VOLUME_MINUTE) ? tempParam.maxVolumeMinute += ADJUSTED_MAX_VOLUME_MINTUE_STEP : tempParam.maxVolumeMinute = MAX_ADJUSTED_MAX_VOLUME_MINUTE;
        itoa(tempParam.maxVolumeMinute,stringAux,10);
        strcat(stringAux, "L/m");
        UI_DisplayMessage(0,1,stringAux);
      } 
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN)) 
      {
        (tempParam.maxVolumeMinute > MIN_ADJUSTED_MAX_VOLUME_MINUTE)? tempParam.maxVolumeMinute -= ADJUSTED_MAX_VOLUME_MINTUE_STEP : tempParam.maxVolumeMinute = MIN_ADJUSTED_MAX_VOLUME_MINUTE;
        itoa(tempParam.maxVolumeMinute,stringAux,10);
        strcat(stringAux, "L/m");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        itoa(tempParam.maxVolumeMinute,stringAux,10);
        strcat(stringAux, "L/m");
        UI_DisplayMessage(0,1,stringAux);
        //UI.maxVolumeMinute = tempParam.maxVolumeMinute;
        uiState = UI_DELAY_END_VOLUME_MINUTE_M;
        UI_Timer(0);
      }
      else if(UI_ButtonDebounce(BUTTON_BACK_PIN))
      {
        if(tempParam.selectedMode == UI_PRESSURE_CONTROL)
        {
          UI_DisplayClear();
          UI_DisplayMessage(0,0,DISPLAY_ADJUSTED_PRESSURE);

          itoa(tempParam.adjustedPressure,stringAux,10);
          strcat(stringAux, "cm.H2O");
          UI_DisplayMessage(0,1,stringAux);

          uiState = UI_SET_ADJUSTED_PRESSURE;
          UI_Timer(0);
        }
        else if(tempParam.selectedMode == UI_VOLUME_CONTROL)
        {
          UI_DisplayClear();
          UI_DisplayMessage(0,0,DISPLAY_TIDAL_VOLUME);

          itoa(tempParam.tidalVolume,stringAux,10);
          strcat(stringAux, "ml");
          UI_DisplayMessage(0,1,stringAux);

          uiState = UI_SET_TIDAL_VOLUME;
          UI_Timer(0);
        }    
      }      
      else if(UI_Timer(TIMEOUT_BLINK))
      {
        itoa(tempParam.maxVolumeMinute,stringAux,10);
        strcat(stringAux, "L/m");
        UI_DisplayMessage(0,1,stringAux);

        uiState = UI_SET_VOLUME_MINUTE_M;
        UI_Timer(0);
      }       
      break;

    case UI_DELAY_END_VOLUME_MINUTE_M:
      if(UI_Timer(TIMEOUT_SHOW_SELECTED_PARAM))
      {
          UI_DisplayClear();
          UI_DisplayMessage(0,0,DISPLAY_VOLUME_MINUTE_m);

          itoa(tempParam.minVolumeMinute,stringAux,10);
          strcat(stringAux, "L/m");
          UI_DisplayMessage(0,1,stringAux);

          uiState = UI_SET_VOLUME_MINUTE_m;
          UI_Timer(0);
      }    
      break;
////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////// MIN VOLUME MINUTE //////////
//////////////////////////////////////////////////////////////////////////////////////////////////////// 
    case UI_SET_VOLUME_MINUTE_m:
      if(UI_ButtonDebounce(BUTTON_UP_PIN)) 
      {
        (tempParam.minVolumeMinute < MAX_ADJUSTED_MIN_VOLUME_MINUTE) ? tempParam.minVolumeMinute += ADJUSTED_MIN_VOLUME_MINTUE_STEP : tempParam.minVolumeMinute = MAX_ADJUSTED_MIN_VOLUME_MINUTE;
        itoa(tempParam.minVolumeMinute,stringAux,10);
        strcat(stringAux, "L/m");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN))
      {
        (tempParam.minVolumeMinute > MIN_ADJUSTED_MIN_VOLUME_MINUTE)? tempParam.minVolumeMinute -= ADJUSTED_MIN_VOLUME_MINTUE_STEP : tempParam.minVolumeMinute = MIN_ADJUSTED_MIN_VOLUME_MINUTE;
        itoa(tempParam.minVolumeMinute,stringAux,10);
        strcat(stringAux, "L/m");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        itoa(tempParam.minVolumeMinute,stringAux,10);
        strcat(stringAux, "L/m");
        UI_DisplayMessage(0,1,stringAux);
        //UI.minVolumeMinute = tempParam.minVolumeMinute;
        uiState = UI_DELAY_END_VOLUME_MINUTE_m;
        UI_Timer(0);
      }
      else if(UI_ButtonDebounce(BUTTON_BACK_PIN))
      {
        UI_DisplayClear();
        UI_DisplayMessage(0,0,DISPLAY_VOLUME_MINUTE_M);
        
        itoa(tempParam.maxVolumeMinute,stringAux,10);
        strcat(stringAux, "L/m");
        UI_DisplayMessage(0,1,stringAux);

        uiState = UI_SET_VOLUME_MINUTE_M;
        UI_Timer(0);         
      }      
      else if(UI_Timer(TIMEOUT_BLINK))
      {
        uiState = UI_BLINK_VOLUME_MINUTE_m;
        UI_DisplayMessage(0,1,DISPLAY_EMPTY_LINE);
        UI_Timer(0);
      }        
      break;

    case UI_BLINK_VOLUME_MINUTE_m:
      if(UI_ButtonDebounce(BUTTON_UP_PIN))  
      {
        (tempParam.minVolumeMinute < MAX_ADJUSTED_MIN_VOLUME_MINUTE) ? tempParam.minVolumeMinute += ADJUSTED_MIN_VOLUME_MINTUE_STEP : tempParam.minVolumeMinute = MAX_ADJUSTED_MIN_VOLUME_MINUTE;
        itoa(tempParam.minVolumeMinute,stringAux,10);
        strcat(stringAux, "L/m");
        UI_DisplayMessage(0,1,stringAux);
      } 
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN)) 
      {
        (tempParam.minVolumeMinute > MIN_ADJUSTED_MIN_VOLUME_MINUTE)? tempParam.minVolumeMinute -= ADJUSTED_MIN_VOLUME_MINTUE_STEP : tempParam.minVolumeMinute = MIN_ADJUSTED_MIN_VOLUME_MINUTE;
        itoa(tempParam.minVolumeMinute,stringAux,10);
        strcat(stringAux, "L/m");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        itoa(tempParam.minVolumeMinute,stringAux,10);
        strcat(stringAux, "L/m");
        UI_DisplayMessage(0,1,stringAux);
        //UI.minVolumeMinute = tempParam.minVolumeMinute;
        uiState = UI_DELAY_END_VOLUME_MINUTE_m;
        UI_Timer(0);
      }
      else if(UI_ButtonDebounce(BUTTON_BACK_PIN))
      {
        UI_DisplayClear();
        UI_DisplayMessage(0,0,DISPLAY_VOLUME_MINUTE_M);
        
        itoa(tempParam.maxVolumeMinute,stringAux,10);
        strcat(stringAux, "L/m");
        UI_DisplayMessage(0,1,stringAux);

        uiState = UI_SET_VOLUME_MINUTE_M;
        UI_Timer(0);        
      }      
      else if(UI_Timer(TIMEOUT_BLINK))
      {
        itoa(tempParam.minVolumeMinute,stringAux,10);
        strcat(stringAux, "L/m");
        UI_DisplayMessage(0,1,stringAux);

        uiState = UI_SET_VOLUME_MINUTE_m;
        UI_Timer(0);
      }       
      break;

    case UI_DELAY_END_VOLUME_MINUTE_m:
      if(UI_Timer(TIMEOUT_SHOW_SELECTED_PARAM))
      {
          UI_DisplayClear();
          UI_DisplayMessage(0,0,DISPLAY_BPM);

          itoa(tempParam.breathsMinute,stringAux,10);
          strcat(stringAux, " ");
          UI_DisplayMessage(0,1,stringAux);

          uiState = UI_SET_RPM;
          UI_Timer(0);
      }    
      break;

////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////// BREATHS MINUTE /////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////       

    case UI_SET_RPM:
      if(UI_ButtonDebounce(BUTTON_UP_PIN)) 
      {
        (tempParam.breathsMinute < MAX_ADJUSTED_BREATHS_MINUTE) ? tempParam.breathsMinute += ADJUSTED_BREATHS_MINUTE_STEP : tempParam.breathsMinute = MAX_ADJUSTED_BREATHS_MINUTE;
        itoa(tempParam.breathsMinute,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN))
      {
        (tempParam.breathsMinute > MIN_ADJUSTED_BREATHS_MINUTE)? tempParam.breathsMinute -= ADJUSTED_BREATHS_MINUTE_STEP : tempParam.breathsMinute = MIN_ADJUSTED_BREATHS_MINUTE;
        itoa(tempParam.breathsMinute,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        itoa(tempParam.breathsMinute,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
        //UI.breathsMinute = tempParam.breathsMinute;
        uiState = UI_DELAY_END_RPM;
        UI_Timer(0);
      }
      else if(UI_ButtonDebounce(BUTTON_BACK_PIN))
      {
        UI_DisplayClear();
        UI_DisplayMessage(0,0,DISPLAY_VOLUME_MINUTE_m);
        
        itoa(tempParam.maxVolumeMinute,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);

        uiState = UI_SET_VOLUME_MINUTE_m;
        UI_Timer(0);         
      }      
      else if(UI_Timer(TIMEOUT_BLINK))
      {
        uiState = UI_BLINK_RPM;
        UI_DisplayMessage(0,1,DISPLAY_EMPTY_LINE);
        UI_Timer(0);
      }        
      break;

    case UI_BLINK_RPM:
      if(UI_ButtonDebounce(BUTTON_UP_PIN))  
      {
        (tempParam.breathsMinute < MAX_ADJUSTED_BREATHS_MINUTE) ? tempParam.breathsMinute += ADJUSTED_BREATHS_MINUTE_STEP : tempParam.breathsMinute = MAX_ADJUSTED_BREATHS_MINUTE;
        itoa(tempParam.breathsMinute,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
      } 
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN)) 
      {
        (tempParam.breathsMinute > MIN_ADJUSTED_BREATHS_MINUTE)? tempParam.breathsMinute -= ADJUSTED_BREATHS_MINUTE_STEP : tempParam.breathsMinute = MIN_ADJUSTED_BREATHS_MINUTE;
        itoa(tempParam.breathsMinute,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        itoa(tempParam.breathsMinute,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
        //UI.breathsMinute = tempParam.breathsMinute;
        uiState = UI_DELAY_END_RPM;
        UI_Timer(0);
      }
      else if(UI_ButtonDebounce(BUTTON_BACK_PIN))
      {
        UI_DisplayClear();
        UI_DisplayMessage(0,0,DISPLAY_VOLUME_MINUTE_m);
        
        itoa(tempParam.maxVolumeMinute,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);

        uiState = UI_SET_VOLUME_MINUTE_m;
        UI_Timer(0);        
      }      
      else if(UI_Timer(TIMEOUT_BLINK))
      {
        itoa(tempParam.breathsMinute,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);

        uiState = UI_SET_RPM;
        UI_Timer(0);
      }       
      break;

    case UI_DELAY_END_RPM:
      if(UI_Timer(TIMEOUT_SHOW_SELECTED_PARAM))
      {
        UI_DisplayClear();
        UI_DisplayMessage(0,0,DISPLAY_MAX_BPM);

        itoa(tempParam.maxBreathsMinute,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);

        uiState = UI_SET_RPM_M;
        UI_Timer(0);
      }    
      break;

////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////// MAX BREATHS MINUTE /////////
////////////////////////////////////////////////////////////////////////////////////////////////////////         

    case UI_SET_RPM_M:
      if(UI_ButtonDebounce(BUTTON_UP_PIN)) 
      {
        (tempParam.maxBreathsMinute < MAX_ADJUSTED_MAX_BREATHS_MINUTE) ? tempParam.maxBreathsMinute += ADJUSTED_MAX_BREATHS_MINUTE_STEP : tempParam.maxBreathsMinute = MAX_ADJUSTED_MAX_BREATHS_MINUTE;
        itoa(tempParam.maxBreathsMinute,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN))
      {
        (tempParam.maxBreathsMinute > MIN_ADJUSTED_MAX_BREATHS_MINUTE)? tempParam.maxBreathsMinute -= ADJUSTED_MAX_BREATHS_MINUTE_STEP : tempParam.maxBreathsMinute = MIN_ADJUSTED_MAX_BREATHS_MINUTE;
        itoa(tempParam.maxBreathsMinute,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        itoa(tempParam.maxBreathsMinute,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
        //UI.breathsMinute = tempParam.breathsMinute;
        uiState = UI_DELAY_END_RPM_M;
        UI_Timer(0);
      }
      else if(UI_ButtonDebounce(BUTTON_BACK_PIN))
      {
        UI_DisplayClear();
        UI_DisplayMessage(0,0,DISPLAY_BPM);
        
        itoa(tempParam.breathsMinute,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);

        uiState = UI_SET_RPM;
        UI_Timer(0);          
      }      
      else if(UI_Timer(TIMEOUT_BLINK))
      {
        uiState = UI_BLINK_RPM_M;
        UI_DisplayMessage(0,1,DISPLAY_EMPTY_LINE);
        UI_Timer(0);
      }        
      break;

    case UI_BLINK_RPM_M:
      if(UI_ButtonDebounce(BUTTON_UP_PIN))  
      {
        (tempParam.maxBreathsMinute < MAX_ADJUSTED_MAX_BREATHS_MINUTE) ? tempParam.maxBreathsMinute += ADJUSTED_MAX_BREATHS_MINUTE_STEP : tempParam.maxBreathsMinute = MAX_ADJUSTED_MAX_BREATHS_MINUTE;
        itoa(tempParam.maxBreathsMinute,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
      } 
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN)) 
      {
        (tempParam.maxBreathsMinute > MIN_ADJUSTED_MAX_BREATHS_MINUTE)? tempParam.maxBreathsMinute -= ADJUSTED_MAX_BREATHS_MINUTE_STEP : tempParam.maxBreathsMinute = MIN_ADJUSTED_MAX_BREATHS_MINUTE;
        itoa(tempParam.maxBreathsMinute,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        itoa(tempParam.maxBreathsMinute,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
        //UI.breathsMinute = tempParam.breathsMinute;
        uiState = UI_DELAY_END_RPM_M;
        UI_Timer(0);
      }
      else if(UI_ButtonDebounce(BUTTON_BACK_PIN))
      {
        UI_DisplayClear();
        UI_DisplayMessage(0,0,DISPLAY_BPM);
        
        itoa(tempParam.breathsMinute,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);

        uiState = UI_SET_RPM;
        UI_Timer(0);        
      }      
      else if(UI_Timer(TIMEOUT_BLINK))
      {
        itoa(tempParam.maxBreathsMinute,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);

        uiState = UI_SET_RPM_M;
        UI_Timer(0);
      }       
      break;

    case UI_DELAY_END_RPM_M:
      if(UI_Timer(TIMEOUT_SHOW_SELECTED_PARAM))
      {
        UI_DisplayClear();
        UI_DisplayMessage(0,0,DISPLAY_MIN_BPM);

        itoa(tempParam.minBreathsMinute,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);

        uiState = UI_SET_RPM_m;
        UI_Timer(0);
      }    
      break;

////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////// MIN BREATHS MINUTE /////////
////////////////////////////////////////////////////////////////////////////////////////////////////////   

    case UI_SET_RPM_m:
      if(UI_ButtonDebounce(BUTTON_UP_PIN)) 
      {
        (tempParam.minBreathsMinute < MAX_ADJUSTED_MIN_BREATHS_MINUTE) ? tempParam.minBreathsMinute += ADJUSTED_MIN_BREATHS_MINUTE_STEP : tempParam.minBreathsMinute = MAX_ADJUSTED_MIN_BREATHS_MINUTE;
        itoa(tempParam.minBreathsMinute,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN))
      {
        (tempParam.minBreathsMinute > MIN_ADJUSTED_MIN_BREATHS_MINUTE) ? tempParam.minBreathsMinute -= ADJUSTED_MIN_BREATHS_MINUTE_STEP : tempParam.minBreathsMinute = MIN_ADJUSTED_MIN_BREATHS_MINUTE;
        itoa(tempParam.minBreathsMinute,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        itoa(tempParam.minBreathsMinute,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
        //UI.breathsMinute = tempParam.breathsMinute;
        uiState = UI_DELAY_END_RPM_m;
        UI_Timer(0);
      }
      else if(UI_ButtonDebounce(BUTTON_BACK_PIN))
      {
        UI_DisplayClear();
        UI_DisplayMessage(0,0,DISPLAY_MAX_BPM);

        itoa(tempParam.maxBreathsMinute,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);

        uiState = UI_SET_RPM_M;
        UI_Timer(0);      
      }      
      else if(UI_Timer(TIMEOUT_BLINK))
      {
        uiState = UI_BLINK_RPM_m;
        UI_DisplayMessage(0,1,DISPLAY_EMPTY_LINE);
        UI_Timer(0);
      }        
      break;

    case UI_BLINK_RPM_m:
      if(UI_ButtonDebounce(BUTTON_UP_PIN)) 
      {
        (tempParam.minBreathsMinute < MAX_ADJUSTED_MIN_BREATHS_MINUTE) ? tempParam.minBreathsMinute += ADJUSTED_MIN_BREATHS_MINUTE_STEP : tempParam.minBreathsMinute = MAX_ADJUSTED_MIN_BREATHS_MINUTE;
        itoa(tempParam.minBreathsMinute,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN))
      {
        (tempParam.minBreathsMinute > MIN_ADJUSTED_MIN_BREATHS_MINUTE)? tempParam.minBreathsMinute -= ADJUSTED_MIN_BREATHS_MINUTE_STEP : tempParam.minBreathsMinute = MIN_ADJUSTED_MIN_BREATHS_MINUTE;
        itoa(tempParam.minBreathsMinute,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        itoa(tempParam.minBreathsMinute,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
        //UI.breathsMinute = tempParam.breathsMinute;
        uiState = UI_DELAY_END_RPM_m;
        UI_Timer(0);
      }
      else if(UI_ButtonDebounce(BUTTON_BACK_PIN))
      {
        UI_DisplayClear();
        UI_DisplayMessage(0,0,DISPLAY_MAX_BPM);

        itoa(tempParam.maxBreathsMinute,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);

        uiState = UI_SET_RPM_M;
        UI_Timer(0);      
      }      
      else if(UI_Timer(TIMEOUT_BLINK))
      {
        itoa(tempParam.minBreathsMinute,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);

        uiState = UI_SET_RPM_m;
        UI_Timer(0);
      }       
      break;

    case UI_DELAY_END_RPM_m:
      if(UI_Timer(TIMEOUT_SHOW_SELECTED_PARAM))
      {
        UI_DisplayClear();
        UI_DisplayMessage(0,0,DISPLAY_T_I);

        itoa(tempParam.t_i,stringAux,10);
        strcat(stringAux, "%");
        UI_DisplayMessage(0,1,stringAux);

        uiState = UI_SET_T_I;
        UI_Timer(0);
      }    
      break;

////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////// INSPIRATION TIME ///////////
////////////////////////////////////////////////////////////////////////////////////////////////////////   

    case UI_SET_T_I:
      if(UI_ButtonDebounce(BUTTON_UP_PIN)) 
      {
        (tempParam.t_i < MAX_ADJUSTED_INSP_TIME) ? tempParam.t_i += ADJUSTED_INSP_TIME_STEP : tempParam.t_i = MAX_ADJUSTED_INSP_TIME;
        itoa(tempParam.t_i,stringAux,10);
        strcat(stringAux, "%");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN))
      {
        (tempParam.t_i > MIN_ADJUSTED_INSP_TIME)? tempParam.t_i -= ADJUSTED_INSP_TIME_STEP : tempParam.t_i = MIN_ADJUSTED_INSP_TIME;
        itoa(tempParam.t_i,stringAux,10);
        strcat(stringAux, "%");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        itoa(tempParam.t_i,stringAux,10);
        strcat(stringAux, "%");
        UI_DisplayMessage(0,1,stringAux);
        //UI.t_i = tempParam.t_i;
        uiState = UI_DELAY_END_T_I;
        UI_Timer(0);
      }
      else if(UI_ButtonDebounce(BUTTON_BACK_PIN))
      {
        UI_DisplayClear();
        UI_DisplayMessage(0,0,DISPLAY_MIN_BPM);
        
        itoa(tempParam.minBreathsMinute,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);

        uiState = UI_SET_RPM_m;
        UI_Timer(0);          
      }      
      else if(UI_Timer(TIMEOUT_BLINK))
      {
        uiState = UI_BLINK_T_I;
        UI_DisplayMessage(0,1,DISPLAY_EMPTY_LINE);
        UI_Timer(0);
      }        
      break;

    case UI_BLINK_T_I:
      if(UI_ButtonDebounce(BUTTON_UP_PIN))  
      {
        (tempParam.t_i < MAX_ADJUSTED_INSP_TIME) ? tempParam.t_i += ADJUSTED_INSP_TIME_STEP : tempParam.t_i = MAX_ADJUSTED_INSP_TIME;
        itoa(tempParam.t_i,stringAux,10);
        strcat(stringAux, "%");
        UI_DisplayMessage(0,1,stringAux);
      } 
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN)) 
      {
        (tempParam.t_i > MIN_ADJUSTED_INSP_TIME)? tempParam.t_i -= ADJUSTED_INSP_TIME_STEP : tempParam.t_i = MIN_ADJUSTED_INSP_TIME;
        itoa(tempParam.t_i,stringAux,10);
        strcat(stringAux, "%");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        itoa(tempParam.t_i,stringAux,10);
        strcat(stringAux, "%");
        UI_DisplayMessage(0,1,stringAux);
        //UI.t_i = tempParam.t_i;
        uiState = UI_DELAY_END_T_I;
        UI_Timer(0);
      }
      else if(UI_ButtonDebounce(BUTTON_BACK_PIN))
      {
        UI_DisplayClear();
        UI_DisplayMessage(0,0,DISPLAY_BPM);
        
        itoa(tempParam.breathsMinute,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);

        uiState = UI_SET_RPM;
        UI_Timer(0);        
      }      
      else if(UI_Timer(TIMEOUT_BLINK))
      {
        itoa(tempParam.t_i,stringAux,10);
        strcat(stringAux, "%");
        UI_DisplayMessage(0,1,stringAux);

        uiState = UI_SET_T_I;
        UI_Timer(0);
      }       
      break;

    case UI_DELAY_END_T_I:
      if(UI_Timer(TIMEOUT_SHOW_SELECTED_PARAM))
      {
          UI_DisplayClear();
          UI_DisplayMessage(0,0,DISPLAY_T_P);

          itoa(tempParam.t_p,stringAux,10);
          strcat(stringAux, "%");
          UI_DisplayMessage(0,1,stringAux);

          uiState = UI_SET_T_P;
          UI_Timer(0);
      }    
      break;

////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////// PAUSE TIME /////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////         

    case UI_SET_T_P:
      if(UI_ButtonDebounce(BUTTON_UP_PIN)) 
      {
        (tempParam.t_p < MAX_ADJUSTED_PAUSE_TIME) ? tempParam.t_p += ADJUSTED_PAUSE_TIME_STEP : tempParam.t_p = MAX_ADJUSTED_PAUSE_TIME;
        itoa(tempParam.t_p,stringAux,10);
        strcat(stringAux, "%");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN))
      {
        (tempParam.t_p > MIN_ADJUSTED_PAUSE_TIME)? tempParam.t_p -= ADJUSTED_PAUSE_TIME_STEP : tempParam.t_p = MIN_ADJUSTED_PAUSE_TIME;
        itoa(tempParam.t_p,stringAux,10);
        strcat(stringAux, "%");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        itoa(tempParam.t_p,stringAux,10);
        strcat(stringAux, "%");
        UI_DisplayMessage(0,1,stringAux);
        //UI.t_p = tempParam.t_p;
        uiState = UI_DELAY_END_T_P;
        UI_Timer(0);
      }
      else if(UI_ButtonDebounce(BUTTON_BACK_PIN))
      {
        UI_DisplayClear();
        UI_DisplayMessage(0,0,DISPLAY_T_I);
        
        itoa(tempParam.t_i,stringAux,10);
        strcat(stringAux, "%");
        UI_DisplayMessage(0,1,stringAux);

        uiState = UI_SET_T_I;
        UI_Timer(0);          
      }      
      else if(UI_Timer(TIMEOUT_BLINK))
      {
        uiState = UI_BLINK_T_P;
        UI_DisplayMessage(0,1,DISPLAY_EMPTY_LINE);
        UI_Timer(0);
      }        
      break;

    case UI_BLINK_T_P:
      if(UI_ButtonDebounce(BUTTON_UP_PIN))  
      {
        (tempParam.t_p < MAX_ADJUSTED_PAUSE_TIME) ? tempParam.t_p += ADJUSTED_PAUSE_TIME_STEP : tempParam.t_p = MAX_ADJUSTED_PAUSE_TIME;
        itoa(tempParam.t_p,stringAux,10);
        strcat(stringAux, "%");
        UI_DisplayMessage(0,1,stringAux);
      } 
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN)) 
      {
        (tempParam.t_p > MIN_ADJUSTED_PAUSE_TIME)? tempParam.t_p -= ADJUSTED_PAUSE_TIME_STEP : tempParam.t_p = MIN_ADJUSTED_PAUSE_TIME;
        itoa(tempParam.t_p,stringAux,10);
        strcat(stringAux, "%");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        itoa(tempParam.t_p,stringAux,10);
        strcat(stringAux, "%");
        UI_DisplayMessage(0,1,stringAux);
        //UI.t_p = tempParam.t_p;
        uiState = UI_DELAY_END_T_P;
        UI_Timer(0);
      }
      else if(UI_ButtonDebounce(BUTTON_BACK_PIN))
      {
        UI_DisplayClear();
        UI_DisplayMessage(0,0,DISPLAY_T_I);
        
        itoa(tempParam.t_i,stringAux,10);
        strcat(stringAux, "%");
        UI_DisplayMessage(0,1,stringAux);

        uiState = UI_SET_T_I;
        UI_Timer(0);         
      }      
      else if(UI_Timer(TIMEOUT_BLINK))
      {
        itoa(tempParam.t_p,stringAux,10);
        strcat(stringAux, "%");
        UI_DisplayMessage(0,1,stringAux);

        uiState = UI_SET_T_P;
        UI_Timer(0);
      }       
      break;

    case UI_DELAY_END_T_P:
      if(UI_Timer(TIMEOUT_SHOW_SELECTED_PARAM))
      {
          UI_DisplayClear();
          UI_DisplayMessage(0,0,DISPLAY_MAX_PRESSURE);

          itoa(tempParam.maxPressure,stringAux,10);
          strcat(stringAux, "cm.H2O");
          UI_DisplayMessage(0,1,stringAux);

          uiState = UI_SET_MAX_PRESSURE;
          UI_Timer(0);
      }    
      break;

////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////// MAX PRESSURE ///////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////   

    case UI_SET_MAX_PRESSURE:
      if(UI_ButtonDebounce(BUTTON_UP_PIN)) 
      {
        (tempParam.maxPressure < MAX_ADJUSTED_MAX_PRESSURE) ? tempParam.maxPressure += ADJUSTED_MAX_PRESSURE_STEP : tempParam.maxPressure = MAX_ADJUSTED_MAX_PRESSURE;
        itoa(tempParam.maxPressure,stringAux,10);
        strcat(stringAux, "cm.H2O");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN))
      {
        (tempParam.maxPressure > MIN_ADJUSTED_MAX_PRESSURE)? tempParam.maxPressure -= ADJUSTED_MAX_PRESSURE_STEP : tempParam.maxPressure = MIN_ADJUSTED_MAX_PRESSURE;
        itoa(tempParam.maxPressure,stringAux,10);
        strcat(stringAux, "cm.H2O");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        itoa(tempParam.maxPressure,stringAux,10);
        strcat(stringAux, "cm.H2O");
        UI_DisplayMessage(0,1,stringAux);
        //UI.maxPressure = tempParam.maxPressure;
        uiState = UI_DELAY_END_MAX_PRESSURE;
        UI_Timer(0);
      }
      else if(UI_ButtonDebounce(BUTTON_BACK_PIN))
      {
        UI_DisplayClear();
        UI_DisplayMessage(0,0,DISPLAY_T_P);
        
        itoa(tempParam.t_p,stringAux,10);
        strcat(stringAux, "%");
        UI_DisplayMessage(0,1,stringAux);

        uiState = UI_SET_T_P;
        UI_Timer(0);          
      }      
      else if(UI_Timer(TIMEOUT_BLINK))
      {
        uiState = UI_BLINK_MAX_PRESSURE;
        UI_DisplayMessage(0,1,DISPLAY_EMPTY_LINE);
        UI_Timer(0);
      }        
      break;

    case UI_BLINK_MAX_PRESSURE:
      if(UI_ButtonDebounce(BUTTON_UP_PIN))  
      {
        (tempParam.maxPressure < MAX_ADJUSTED_MAX_PRESSURE) ? tempParam.maxPressure += ADJUSTED_MAX_PRESSURE_STEP : tempParam.maxPressure = MAX_ADJUSTED_MAX_PRESSURE;
        itoa(tempParam.maxPressure,stringAux,10);
        strcat(stringAux, "cm.H2O");
        UI_DisplayMessage(0,1,stringAux);
      } 
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN)) 
      {
        (tempParam.maxPressure > MIN_ADJUSTED_MAX_PRESSURE)? tempParam.maxPressure -= ADJUSTED_MAX_PRESSURE_STEP : tempParam.maxPressure = MIN_ADJUSTED_MAX_PRESSURE;
        itoa(tempParam.maxPressure,stringAux,10);
        strcat(stringAux, "cm.H2O");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        itoa(tempParam.maxPressure,stringAux,10);
        strcat(stringAux, "cm.H2O");
        UI_DisplayMessage(0,1,stringAux);
        //UI.maxPressure = tempParam.maxPressure;
        uiState = UI_DELAY_END_MAX_PRESSURE;
        UI_Timer(0);
      }
      else if(UI_ButtonDebounce(BUTTON_BACK_PIN))
      {
        UI_DisplayClear();
        UI_DisplayMessage(0,0,DISPLAY_T_P);
        
        itoa(tempParam.t_p,stringAux,10);
        strcat(stringAux, "%");
        UI_DisplayMessage(0,1,stringAux);

        uiState = UI_SET_T_P;
        UI_Timer(0);       
      }      
      else if(UI_Timer(TIMEOUT_BLINK))
      {
        itoa(tempParam.maxPressure,stringAux,10);
        strcat(stringAux, "cm.H2O");
        UI_DisplayMessage(0,1,stringAux);

        uiState = UI_SET_MAX_PRESSURE;
        UI_Timer(0);
      }       
      break;

    case UI_DELAY_END_MAX_PRESSURE:
      if(UI_Timer(TIMEOUT_SHOW_SELECTED_PARAM))
      {
          UI_DisplayClear();
          UI_DisplayMessage(0,0,DISPLAY_MIN_PRESSURE);

          itoa(tempParam.minPressure,stringAux,10);
          strcat(stringAux, "cm.H2O");
          UI_DisplayMessage(0,1,stringAux);

          uiState = UI_SET_MIN_PRESSURE;
          UI_Timer(0);
      }    
      break;

////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////// MIN PRESSURE ///////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////    

    case UI_SET_MIN_PRESSURE:
      if(UI_ButtonDebounce(BUTTON_UP_PIN)) 
      {
        (tempParam.minPressure < MAX_ADJUSTED_MIN_PRESSURE) ? tempParam.minPressure += ADJUSTED_MIN_PRESSURE_STEP : tempParam.minPressure = MAX_ADJUSTED_MIN_PRESSURE;
        itoa(tempParam.minPressure,stringAux,10);
        strcat(stringAux, "cm.H2O");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN))
      {
        (tempParam.minPressure > MIN_ADJUSTED_MIN_PRESSURE)? tempParam.minPressure -= ADJUSTED_MIN_PRESSURE_STEP : tempParam.minPressure = MIN_ADJUSTED_MIN_PRESSURE;
        itoa(tempParam.minPressure,stringAux,10);
        strcat(stringAux, "cm.H2O");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        itoa(tempParam.minPressure,stringAux,10);
        strcat(stringAux, "cm.H2O");
        UI_DisplayMessage(0,1,stringAux);
        //UI.maxPressure = tempParam.maxPressure;
        uiState = UI_DELAY_END_MIN_PRESSURE;
        UI_Timer(0);
      }
      else if(UI_ButtonDebounce(BUTTON_BACK_PIN))
      {
        UI_DisplayClear();
        UI_DisplayMessage(0,0,DISPLAY_MAX_PRESSURE);

        itoa(tempParam.maxPressure,stringAux,10);
        strcat(stringAux, "cm.H2O");
        UI_DisplayMessage(0,1,stringAux);

        uiState = UI_SET_MAX_PRESSURE;
        UI_Timer(0);         
      }      
      else if(UI_Timer(TIMEOUT_BLINK))
      {
        uiState = UI_BLINK_MIN_PRESSURE;
        UI_DisplayMessage(0,1,DISPLAY_EMPTY_LINE);
        UI_Timer(0);
      }        
      break;

    case UI_BLINK_MIN_PRESSURE:
      if(UI_ButtonDebounce(BUTTON_UP_PIN)) 
      {
        (tempParam.minPressure < MAX_ADJUSTED_MIN_PRESSURE) ? tempParam.minPressure += ADJUSTED_MIN_PRESSURE_STEP : tempParam.minPressure = MAX_ADJUSTED_MIN_PRESSURE;
        itoa(tempParam.minPressure,stringAux,10);
        strcat(stringAux, "cm.H2O");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN))
      {
        (tempParam.minPressure > MIN_ADJUSTED_MIN_PRESSURE)? tempParam.minPressure -= ADJUSTED_MIN_PRESSURE_STEP : tempParam.minPressure = MIN_ADJUSTED_MIN_PRESSURE;
        itoa(tempParam.minPressure,stringAux,10);
        strcat(stringAux, "cm.H2O");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        itoa(tempParam.minPressure,stringAux,10);
        strcat(stringAux, "cm.H2O");
        UI_DisplayMessage(0,1,stringAux);
        //UI.maxPressure = tempParam.maxPressure;
        uiState = UI_DELAY_END_MIN_PRESSURE;
        UI_Timer(0);
      }
      else if(UI_ButtonDebounce(BUTTON_BACK_PIN))
      {
        UI_DisplayClear();
        UI_DisplayMessage(0,0,DISPLAY_MAX_PRESSURE);

        itoa(tempParam.maxPressure,stringAux,10);
        strcat(stringAux, "cm.H2O");
        UI_DisplayMessage(0,1,stringAux);

        uiState = UI_SET_MAX_PRESSURE;
        UI_Timer(0);         
      }        
      else if(UI_Timer(TIMEOUT_BLINK))
      {
        itoa(tempParam.minPressure,stringAux,10);
        strcat(stringAux, "cm.H2O");
        UI_DisplayMessage(0,1,stringAux);

        uiState = UI_SET_MIN_PRESSURE;
        UI_Timer(0);
      }       
      break;

    case UI_DELAY_END_MIN_PRESSURE:
      if(UI_Timer(TIMEOUT_SHOW_SELECTED_PARAM))
      {
          UI_DisplayClear();
          UI_DisplayMessage(0,0,DISPLAY_TRP);

          itoa(tempParam.TrP,stringAux,10);
          strcat(stringAux, " ");
          UI_DisplayMessage(0,1,stringAux);

          uiState = UI_SET_TRP;
          UI_Timer(0);
      }    
      break;

////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////// TRIGGER PRESSURE ///////////
////////////////////////////////////////////////////////////////////////////////////////////////////////  

    case UI_SET_TRP:
      if(UI_ButtonDebounce(BUTTON_UP_PIN)) 
      {
        (tempParam.TrP < MAX_ADJUSTED_TRIGGER_PRESSURE) ? tempParam.TrP += ADJUSTED_TRIGGER_PRESSURE_STEP : tempParam.TrP = MAX_ADJUSTED_TRIGGER_PRESSURE;
        itoa(tempParam.TrP,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN))
      {
        (tempParam.TrP > MIN_ADJUSTED_TRIGGER_PRESSURE)? tempParam.TrP -= ADJUSTED_TRIGGER_PRESSURE_STEP : tempParam.TrP = MIN_ADJUSTED_TRIGGER_PRESSURE;
        itoa(tempParam.TrP,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        itoa(tempParam.TrP,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
        //UI.TrP = tempParam.TrP;
        uiState = UI_DELAY_END_TRP;
        UI_Timer(0);
      }
      else if(UI_ButtonDebounce(BUTTON_BACK_PIN))
      {
        UI_DisplayClear();
        UI_DisplayMessage(0,0,DISPLAY_MIN_PRESSURE);

        itoa(tempParam.minPressure,stringAux,10);
        strcat(stringAux, "cm.H2O");
        UI_DisplayMessage(0,1,stringAux);

        uiState = UI_SET_MIN_PRESSURE;
        UI_Timer(0);          
      }      
      else if(UI_Timer(TIMEOUT_BLINK))
      {
        uiState = UI_BLINK_TRP;
        UI_DisplayMessage(0,1,DISPLAY_EMPTY_LINE);
        UI_Timer(0);
      }        
      break;

    case UI_BLINK_TRP:
      if(UI_ButtonDebounce(BUTTON_UP_PIN))  
      {
        (tempParam.TrP < MAX_ADJUSTED_TRIGGER_PRESSURE) ? tempParam.TrP += ADJUSTED_TRIGGER_PRESSURE_STEP : tempParam.TrP = MAX_ADJUSTED_TRIGGER_PRESSURE;
        itoa(tempParam.TrP,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
      } 
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN)) 
      {
        (tempParam.TrP > MIN_ADJUSTED_TRIGGER_PRESSURE)? tempParam.TrP -= ADJUSTED_TRIGGER_PRESSURE_STEP : tempParam.TrP = MIN_ADJUSTED_TRIGGER_PRESSURE;
        itoa(tempParam.TrP,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        itoa(tempParam.TrP,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
        //UI.TrP = tempParam.TrP;
        uiState = UI_DELAY_END_TRP;
        UI_Timer(0);
      }
      else if(UI_ButtonDebounce(BUTTON_BACK_PIN))
      {
        UI_DisplayClear();
        UI_DisplayMessage(0,0,DISPLAY_MAX_PRESSURE);

        itoa(tempParam.maxPressure,stringAux,10);
        strcat(stringAux, "cm.H2O");
        UI_DisplayMessage(0,1,stringAux);

        uiState = UI_SET_MAX_PRESSURE;
        UI_Timer(0);         
      }      
      else if(UI_Timer(TIMEOUT_BLINK))
      {
        itoa(tempParam.TrP,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);

        uiState = UI_SET_TRP;
        UI_Timer(0);
      }       
      break;

    case UI_DELAY_END_TRP:
      if(UI_Timer(TIMEOUT_SHOW_SELECTED_PARAM))
      {
          UI_DisplayClear();
          UI_DisplayMessage(0,0,DISPLAY_CONFIRMATION);
          UI_DisplayMessage(0,1,DISPLAY_CONFIRMATION_OPTIONS);
          UI_DisplayMessage(0,1,stringAux);

          uiState = UI_CONFIRM_CONFIG_PARAMETERS;
      }    
      break;

////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////// DEFAULT PARAMETERS /////////
////////////////////////////////////////////////////////////////////////////////////////////////////////        

    case UI_SET_DEFAULT_PARAMETERS:
      if(UI_Timer(TIMEOUT_SHOW_SELECTED_PARAM))
      {
        UI_DisplayClear();
        (tempParam.selectedMode == UI_AUTOMATIC_CONTROL) ?
          UI_DisplayMessage(0,0,DISPLAY_AUTO_CONFIRMATION) : UI_DisplayMessage(0,0,DISPLAY_CONFIRMATION);
        UI_DisplayMessage(0,1,DISPLAY_CONFIRMATION_OPTIONS);
        UI_DisplayMessage(0,1,stringAux);
        uiState = UI_CONFIRM_CONFIG_PARAMETERS;
      } 
      break;

////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////// CONFIRMATION ///////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////  

    case UI_CONFIRM_CONFIG_PARAMETERS:
      if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        (tempParam.selectedMode == UI_AUTOMATIC_CONTROL) ?
          UI_LoadParam(1) : UI_LoadParam(0);

        UI.setUpComplete = true;

        uiState = UI_SET_MODE_AUTO;

        UI_DisplayClear();
        UI_Timer(0);

        // Debug
        /*
        Serial.print("MODO:               ");
        switch(UI.selectedMode)
        {
          case 0:
            Serial.println("VOLUMEN");
            Serial.print("Volumen Tidal:      ");
            Serial.println(UI.tidalVolume);
            break;
          case 1:
            Serial.println("PRESION");
            Serial.print("Presión Ajustada:   ");
            Serial.println(UI.adjustedPressure);
            break;
          case 2:
            Serial.println("AUTOMATICO");
            Serial.print("Volumen Tidal:      ");
            Serial.println(UI.tidalVolume);
            break;
        }

        Serial.print("Volumen minuto Máx: ");
        Serial.println(UI.maxVolumeMinute);
        Serial.print("Volumen minuto mín: ");
        Serial.println(UI.minVolumeMinute);
        Serial.print("RPM:                ");
        Serial.println(UI.breathsMinute);
        Serial.print("MAX RPM:            ");
        Serial.println(UI.maxBreathsMinute);
        Serial.print("MIN RPM:            ");
        Serial.println(UI.minBreathsMinute);
        Serial.print("TI%                 ");
        Serial.println(UI.t_i);
        Serial.print("TP%                 ");
        Serial.println(UI.t_p);
        Serial.print("Presión Máx:        ");
        Serial.println(UI.maxPressure);
        Serial.print("Presión Mín:        ");
        Serial.println(UI.minPressure);        
        Serial.print("TrP:                ");
        Serial.println(UI.TrP);
        */
      }
      
      else if(UI_ButtonDebounce(BUTTON_BACK_PIN))
      {
        if(tempParam.selectedMode != UI_AUTOMATIC_CONTROL)
        {
          UI_DisplayClear();
          UI_DisplayMessage(0,0,DISPLAY_TRP);

          itoa(tempParam.TrP,stringAux,10);
          strcat(stringAux, " ");
          UI_DisplayMessage(0,1,stringAux);

          uiState = UI_SET_TRP;
          UI_Timer(0); 
        }
        else 
        {
          UI_DisplayClear();
          UI_DisplayMessage(0,0,DISPLAY_SELECT_MODE);
          UI_DisplayMessage(0,1,DISPLAY_AUTO_MODE);

          uiState = UI_SET_MODE_AUTO;
          UI_Timer(0);
        }
      }    
      break;

    default:
      uiState = UI_SET_MODE_AUTO;
      break;
  }
}

void UI_LoadParam(uint8_t param)
{
  switch(param)
  {
    case 0:
      UI.selectedMode     = tempParam.selectedMode;
      UI.t_i              = tempParam.t_i;
      UI.t_p              = tempParam.t_p;
      UI.maxPressure      = tempParam.maxPressure;
      UI.minPressure      = tempParam.minPressure;
      UI.breathsMinute    = tempParam.breathsMinute;
      UI.tidalVolume      = tempParam.tidalVolume;
      UI.TrP              = tempParam.TrP;
      UI.maxVolumeMinute  = tempParam.maxVolumeMinute;
      UI.minVolumeMinute  = tempParam.minVolumeMinute;
      UI.adjustedPressure = tempParam.adjustedPressure;
      UI.maxBreathsMinute = tempParam.maxBreathsMinute;
      UI.minBreathsMinute = tempParam.minBreathsMinute;    
      break;

    case 1:
      tempParam.selectedMode     = DEFAULT_SELECTED_MODE;
      tempParam.tidalVolume      = DEFAULT_TIDAL_VOLUME;
      tempParam.maxVolumeMinute  = DEFAULT_MAX_VOLUME_MINUTE;
      tempParam.minVolumeMinute  = DEFAULT_MIN_VOLUME_MINUTE;
      tempParam.breathsMinute    = DEFAULT_BREATHS_MINUTE;
      tempParam.maxBreathsMinute = DEFAULT_MAX_BREATHS_MINUTE;
      tempParam.minBreathsMinute = DEFAULT_MIN_BREATHS_MINUTE;
      tempParam.t_i              = DEFAULT_T_INSP;
      tempParam.t_p              = DEFAULT_T_PAUSE;
      tempParam.maxPressure      = DEFAULT_MAX_PRESSURE;
      tempParam.minPressure      = DEFAULT_MIN_PRESSURE;
      tempParam.TrP              = DEFAULT_TRP;
      tempParam.adjustedPressure = DEFAULT_ADJUSTED_PRESSURE;
      UI.selectedMode     = tempParam.selectedMode;
      UI.t_i              = tempParam.t_i;
      UI.t_p              = tempParam.t_p;
      UI.maxPressure      = tempParam.maxPressure;
      UI.minPressure      = tempParam.minPressure;
      UI.breathsMinute    = tempParam.breathsMinute;
      UI.tidalVolume      = tempParam.tidalVolume;
      UI.TrP              = tempParam.TrP;
      UI.maxVolumeMinute  = tempParam.maxVolumeMinute;
      UI.minVolumeMinute  = tempParam.minVolumeMinute;
      UI.adjustedPressure = tempParam.adjustedPressure;
      UI.maxBreathsMinute = tempParam.maxBreathsMinute;
      UI.minBreathsMinute = tempParam.minBreathsMinute;        
      break;
  }

}

void UI_ShowParametersTask()
{
  char stringAux1[16], 
       stringAux2[8];

  static UI_ShowParametersStates_e actualState;

  stringAux1[0] = 0;
  stringAux2[0] = 0;

  switch(spState)
  {
    case UI_SCREEN_1:
      spState = UI_DELAY_SP; 
      actualState = UI_SCREEN_1;
      UI_Timer2(0);

      if((UI.selectedMode == UI_AUTOMATIC_CONTROL) || ((UI.selectedMode == UI_VOLUME_CONTROL)))
      {
        strcat(stringAux1,"VOL");
      }
      else if(UI.selectedMode == UI_PRESSURE_CONTROL)
      {
        strcat(stringAux1,"PRE");
      }
      dtostrf(showParam.volume,2,1,stringAux2);
      //strcat(stringAux2,"ml ");
      UI_DisplayParameters(DISPLAY_S_MODE,DISPLAY_R_SND_VOL,stringAux1,4,stringAux2,11);

      if(UI_ButtonDebounce(BUTTON_UP_PIN))
      {
        UI_DisplayClear();
        spState = UI_SCREEN_8;
      } 
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN))
      {
        UI_DisplayClear();
        spState = UI_SCREEN_2;
      } 
      break;

    case UI_SCREEN_2:
      spState = UI_DELAY_SP;     
      actualState = UI_SCREEN_2;
      UI_Timer2(0);

      if((UI.selectedMode == UI_AUTOMATIC_CONTROL) || ((UI.selectedMode == UI_VOLUME_CONTROL)))
      {
        itoa(UI.tidalVolume,stringAux1,10);
        //strcat(stringAux1,"ml ");
        dtostrf(showParam.volumeMinute,2,1,stringAux2);
        //strcat(stringAux2,"L/m ");
        UI_DisplayParameters(DISPLAY_S_TIDAL_VOL,DISPLAY_R_VOL_MIN,stringAux1,4,stringAux2,12);
      }
      else if(UI.selectedMode == UI_PRESSURE_CONTROL)
      {
        itoa(UI.adjustedPressure,stringAux1,10);
        dtostrf(showParam.volumeMinute,2,1,stringAux2);
        //strcat(stringAux2,"L/m ");
        UI_DisplayParameters(DISPLAY_S_ADJ_PRESS,DISPLAY_R_VOL_MIN,stringAux1,4,stringAux2,9);
      }

      if(UI_ButtonDebounce(BUTTON_UP_PIN))
      {
        UI_DisplayClear();
        spState = UI_SCREEN_1;
      } 
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN))
      {
        UI_DisplayClear();
        spState = UI_SCREEN_3;
      }    
      break;

    case UI_SCREEN_3:
      spState = UI_DELAY_SP; 
      actualState = UI_SCREEN_3;
      UI_Timer2(0);

      dtostrf(showParam.pressure,2,1,stringAux1);
      dtostrf(showParam.PEEP,2,1,stringAux2);
      UI_DisplayParameters(DISPLAY_R_PRESSURE,DISPLAY_R_PEEP,stringAux1,4,stringAux2,12);

      if(UI_ButtonDebounce(BUTTON_UP_PIN))
      {
        UI_DisplayClear();
        spState = UI_SCREEN_2;
      } 
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN))
      {
        UI_DisplayClear();
        spState = UI_SCREEN_4;
      }     
      break;

    case UI_SCREEN_4:
      spState = UI_DELAY_SP; 
      actualState = UI_SCREEN_4;
      UI_Timer2(0);

      itoa(UI.maxPressure,stringAux1,10);
      dtostrf(showParam.peakPressure,2,1,stringAux2);
      UI_DisplayParameters(DISPLAY_S_MAX_PRESS,DISPLAY_R_PEAK_PRESS,stringAux1,4,stringAux2,12);

      if(UI_ButtonDebounce(BUTTON_UP_PIN))
      {
        UI_DisplayClear();
        spState = UI_SCREEN_3;
      } 
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN))
      {
        UI_DisplayClear();
        spState = UI_SCREEN_5;
      }    
      break;

    case UI_SCREEN_5:
      spState = UI_DELAY_SP; 
      actualState = UI_SCREEN_5;
      UI_Timer2(0);

      itoa(UI.breathsMinute,stringAux1,10);
      dtostrf(showParam.breathsMinute,2,0,stringAux2);
      UI_DisplayParameters(DISPLAY_S_BPM,DISPLAY_R_BPM,stringAux1,4,stringAux2,13);

      if(UI_ButtonDebounce(BUTTON_UP_PIN))
      {
        UI_DisplayClear();
        spState = UI_SCREEN_4;
      } 
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN))
      {
        UI_DisplayClear();
        spState = UI_SCREEN_6;
      }    
      break;

    case UI_SCREEN_6:
      spState = UI_DELAY_SP; 
      actualState = UI_SCREEN_6;
      UI_Timer2(0);

      itoa(UI.t_i,stringAux1,10);
      itoa(UI.t_p,stringAux2,10);
      UI_DisplayParameters(DISPLAY_S_TI,DISPLAY_S_TP,stringAux1,4,stringAux2,12);

      if(UI_ButtonDebounce(BUTTON_UP_PIN))
      {
        UI_DisplayClear();
        spState = UI_SCREEN_5;
      } 
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN))
      {
        UI_DisplayClear();
        spState = UI_SCREEN_7;
      }    
      break;

    case UI_SCREEN_7:
      spState = UI_DELAY_SP; 
      actualState = UI_SCREEN_7;
      UI_Timer2(0);

      itoa(UI.TrP,stringAux1,10);
      dtostrf(showParam.dynamicCompliance,3,2,stringAux2);
      UI_DisplayParameters(DISPLAY_S_TRP,DISPLAY_R_LUNG_COMP,stringAux1,4,stringAux2,12);
      if(UI_ButtonDebounce(BUTTON_UP_PIN))
      {
        UI_DisplayClear();
        spState = UI_SCREEN_6;
      } 
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN))
      {
        UI_DisplayClear();
        spState = UI_SCREEN_8;
      }    
      break;      

    case UI_SCREEN_8:
      spState = UI_DELAY_SP;  
      actualState = UI_SCREEN_8;
      UI_Timer2(0);

      itoa(working_time.hour,stringAux2,10);
      strcat(stringAux1,stringAux2);
      strcat(stringAux1,"h ");

      itoa(working_time.mnt,stringAux2,10);
      strcat(stringAux1,stringAux2);
      strcat(stringAux1,"m ");

      itoa(working_time.scn,stringAux2,10);
      strcat(stringAux1,stringAux2);
      strcat(stringAux1,"s ");

      UI_DisplayMessage(0,0,DISPLAY_VENTILATION_TIME);   
      UI_DisplayMessage(0,1,stringAux1);  

      if(UI_ButtonDebounce(BUTTON_UP_PIN))
      {
        UI_DisplayClear();
        spState = UI_SCREEN_7;
      } 
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN))
      {
        UI_DisplayClear();
        spState = UI_SCREEN_1;
      }    
      break;      

    case UI_DELAY_SP:
      if(UI_Timer2(TIMEOUT_UPDATE_SCREEN))
      {
        UI_UpdateControlParam();
        spState = actualState;
      }
      else if(UI_ButtonDebounce(BUTTON_UP_PIN))
      {
        UI_DisplayClear();

        if(actualState == UI_SCREEN_1)
        {
          spState = UI_SCREEN_8;
        }
        else if(actualState == UI_SCREEN_2)
        {
          spState = UI_SCREEN_1;
        }
        else if(actualState == UI_SCREEN_3)
        {
          spState = UI_SCREEN_2;
        }
        else if(actualState == UI_SCREEN_4)
        {
          spState = UI_SCREEN_3;
        }
        else if(actualState == UI_SCREEN_5)
        {
          spState = UI_SCREEN_4;
        }
        else if(actualState == UI_SCREEN_6)
        {
          spState = UI_SCREEN_5;
        }
        else if(actualState == UI_SCREEN_7)
        {
          spState = UI_SCREEN_6;
        }
        else if(actualState == UI_SCREEN_8)
        {
          spState = UI_SCREEN_7;
        }
      } 
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN))
      {
        UI_DisplayClear();
        if(actualState == UI_SCREEN_1)
        {
          spState = UI_SCREEN_2;
        }
        else if(actualState == UI_SCREEN_2)
        {
          spState = UI_SCREEN_3;
        }
        else if(actualState == UI_SCREEN_3)
        {
          spState = UI_SCREEN_4;
        }
        else if(actualState == UI_SCREEN_4)
        {
          spState = UI_SCREEN_5;
        }
        else if(actualState == UI_SCREEN_5)
        {
          spState = UI_SCREEN_6;
        }
        else if(actualState == UI_SCREEN_6)
        {
          spState = UI_SCREEN_7;
        }
        else if(actualState == UI_SCREEN_7)
        {
          spState = UI_SCREEN_8;
        }
        else if(actualState == UI_SCREEN_8)
        {
          spState = UI_SCREEN_1;
        }
      } 
      break;

    default:
      spState = UI_SCREEN_1;
      break;
  }
}

void UI_MedicalAlarmTask()
{
  static uint8_t lastActiveAlarmQty;

  // Display tasks
  switch(alarmMedicalState)
  {
    case UI_ALARM_MEDICAL_IDLE:
      alarmMedicalState = UI_ALARM_MEDICAL_SHOW;
      actualAlarm = UI_MedAlarmNextActive(ALARM_MEDICAL_OFF); // Get first alarm

      digitalWrite(LED_MEDICAL_ALARM_PIN, HIGH);
      break;

    case UI_ALARM_MEDICAL_SHOW:
      alarmMedicalState = UI_ALARM_MEDICAL_WAIT_FOR_ACK;
      
      UI_MedicalAlarmDisplay(actualAlarm);
      lastActiveAlarmQty = UI_MedAlarmActiveQty();
      break;

    case UI_ALARM_MEDICAL_WAIT_FOR_ACK:      
      if (lastActiveAlarmQty != UI_MedAlarmActiveQty())
      {
        alarmMedicalState = UI_ALARM_MEDICAL_SHOW;
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        UI_MedAlarmActiveTurnOff(actualAlarm); 
        
        if (UI_ActiveMedicalAlarms()) 
        {
          alarmMedicalState = UI_ALARM_MEDICAL_SHOW;
          actualAlarm = UI_MedAlarmNextActive(actualAlarm);
          
          //Serial.print(UI_MedAlarmActiveQty()); Serial.print(" "); Serial.println((uint8_t)actualAlarm);
        }
        else
        {
          alarmMedicalState = UI_ALARM_MEDICAL_IDLE;
          digitalWrite(LED_MEDICAL_ALARM_PIN, LOW);
          UI_DisplayClear();
        }         
      } 
      else if (UI_MedAlarmActiveQty()>1)
      {
        if(UI_ButtonDebounce(BUTTON_UP_PIN))
        {
          alarmMedicalState = UI_ALARM_MEDICAL_SHOW;
          actualAlarm = UI_MedAlarmNextActive(actualAlarm); 
          //Serial.print("FN "); Serial.println((uint8_t)actualAlarm);
        }  
        else if(UI_ButtonDebounce(BUTTON_DOWN_PIN))
        {
          alarmMedicalState = UI_ALARM_MEDICAL_SHOW;
          actualAlarm = UI_MedAlarmPrevActive(actualAlarm); 
          //Serial.print("FP "); Serial.println((uint8_t)actualAlarm);
        }  
      } 
      break;          

    default:
      alarmMedicalState = UI_ALARM_MEDICAL_IDLE;
      break;
  }

  // Buzzer tasks
  if (UI_AlarmBuzzerTimer(1000)) // todo
  {
    UI_AlarmBuzzerTimer(0);

    tone(BUZZER_ALARM_PIN, 1000, 150);
  }
}

void UI_SystemAlarmTask()
{
  static uint8_t actualAlarm;

  // Display tasks
  switch(alarmSystemState)
  {
    case UI_ALARM_SYSTEM_IDLE:
      alarmSystemState = UI_ALARM_SYSTEM_SHOW_ERROR_CODE;
      actualAlarm = UI_SysAlarmNextActive(ALARM_SYSTEM_OFF); // Get first system alarm   

      digitalWrite(LED_MEDICAL_ALARM_PIN, HIGH);     
      break;

    case UI_ALARM_SYSTEM_SHOW_ERROR_CODE:
      if (UI_AlarmDisplayTimer(1000))
      {
        UI_AlarmDisplayTimer(0);
        alarmSystemState = UI_ALARM_SYSTEM_SHOW_CALL_SUPPLIER;
      
        UI_SystemAlarmDisplayErrorCode(actualAlarm);
      }
      break;

    case UI_ALARM_SYSTEM_SHOW_CALL_SUPPLIER:    
      if (UI_AlarmDisplayTimer(1000))
      {
        UI_AlarmDisplayTimer(0);
        alarmSystemState = UI_ALARM_SYSTEM_SHOW_ERROR_CODE;

        UI_SystemAlarmDisplayCallSupplier(actualAlarm);
      }    
      break;        

    default:
      alarmSystemState = UI_ALARM_SYSTEM_IDLE;
      break;
  }

  // Buzzer tasks
  //if (UI_AlarmBuzzerTimer(1000)) //todo
  //{
  //  UI_AlarmBuzzerTimer(0); //todo
  //  
  //  tone(BUZZER_ALARM_PIN, 500, 200); //todo
  //}  

  if (ALARM.mute)
  {
    ALARM.mute = false;

    UI_AlarmMuteTimer(0);
  }
  else if (UI_AlarmMuteTimer(5000))
  {    
    if (UI_AlarmBuzzerTimer(1000)) // todo
    {
      UI_AlarmBuzzerTimer(0);

      tone(BUZZER_ALARM_PIN, 1000, 150);
    }
  }

}

void UI_SetMedicalAlarm(uint8_t alarm, float triggerValue, float thresholdValue)
{
  if (ALARM.enable)
  {
    ALARM.newMedicalEvent = true; 
    Serial.print(alarm); Serial.print('\t'); Serial.print(triggerValue); Serial.print('\t'); Serial.println(thresholdValue);
    ALARM.medical[alarm].isActive = true;    
    ALARM.medical[alarm].triggerValue = triggerValue;
    ALARM.medical[alarm].thresholdValue = thresholdValue;
  }
}

void UI_SetSystemAlarm(uint8_t alarm)
{
  if (ALARM.enable)
  {
    ALARM.newSystemEvent = true; 

    ALARM.system[alarm].isActive = true;    
  }
}

void UI_MedicalAlarmDisplay(uint8_t alarm)
{
  char strLine[DISPLAY_COLUMNS+1], strAlarmIdx[3], strAlarmQty[3];

  // Clear
  UI_DisplayClear();

  // First line
  strcpy(strLine, "Alarma ");  // todo macro
  itoa(UI_MedAlarmActiveIdx(alarm),strAlarmIdx,10);
  strcat(strLine, strAlarmIdx);
  strcat(strLine, "/");
  itoa(UI_MedAlarmActiveQty(),strAlarmQty,10);
  strcat(strLine, strAlarmQty);

  UI_DisplayMessage(0,0,strLine);

  // Second line
  char strAlarmTrg[5], strAlarmThld[5];

  // Trigger
  if (ALARM.medical[alarm].triggerValue>=10)
  {
    dtostrf(ALARM.medical[alarm].triggerValue,3,0,strAlarmTrg);
  }
  else
  {
    dtostrf(ALARM.medical[alarm].triggerValue,1,1,strAlarmTrg);
  }

  // Threshold
  if (ALARM.medical[alarm].thresholdValue>=10)
  {
    dtostrf(ALARM.medical[alarm].thresholdValue,3,0,strAlarmThld);
  }
  else
  {
    dtostrf(ALARM.medical[alarm].thresholdValue,1,1,strAlarmThld);
  }

  strcpy(strLine, alarmTriggerMedicalMessage[alarm]);  
  strcat(strLine, strAlarmTrg);

  UI_DisplayMessage(0,1,strLine);

  strcpy(strLine, alarmThresholdMedicalMessage[alarm]);
  strcat(strLine, strAlarmThld);

  UI_DisplayMessage(7,1,strLine);
}

void UI_SystemAlarmDisplayErrorCode(uint8_t alarm)
{
  char strLine[DISPLAY_COLUMNS+1]; 
  // First line
  strcpy(strLine, "Error:          "); // todo macro

  UI_DisplayMessage(0,0,strLine);

  // Second line
  UI_DisplayMessage(0,1,alarmSystemMessage[alarm]);
}

void UI_SystemAlarmDisplayCallSupplier(uint8_t alarm)
{
  UI_DisplayMessage(0,0,"LLamar a        ");  // todo macro
  UI_DisplayMessage(0,1,"servicio tecnico");  // todo macro
}

bool UI_ActiveMedicalAlarms()
{
  // Check for a new event and short circuit
  if (ALARM.newMedicalEvent)
  {
    ALARM.newMedicalEvent = false;
    return true;
  }

  // Cher for active alarms
  uint8_t alarmIndex;
  for (alarmIndex=0;alarmIndex<ALARM_MEDICAL_QTY;alarmIndex++)
  {
	  if (ALARM.medical[alarmIndex].isActive) 
      return true;
  } 
  return false;
}

bool UI_ActiveSystemAlarms()
{
  // Check for a new event and short circuit
  if (ALARM.newSystemEvent)
  {
    ALARM.newSystemEvent = false;
    return true;
  }

  // Check for active alarms
  uint8_t alarmIndex;
  for (alarmIndex=0;alarmIndex<ALARM_SYSTEM_QTY;alarmIndex++)
  {
	  if (ALARM.system[alarmIndex].isActive) 
      return true;
  } 
  return false;
}

uint8_t UI_MedAlarmNextActive(uint8_t actualAlarm)
{
	uint8_t alarmIndex, activeAlarmIndex;
	for (alarmIndex = 1;alarmIndex<ALARM_MEDICAL_QTY+1;alarmIndex++)
	{
		activeAlarmIndex = (actualAlarm+alarmIndex)%ALARM_MEDICAL_QTY;
		if (ALARM.medical[activeAlarmIndex].isActive) 
      return activeAlarmIndex; 
	}
  return activeAlarmIndex; 
}

uint8_t UI_SysAlarmNextActive(uint8_t actualAlarm)
{
	uint8_t alarmIndex, activeAlarmIndex;
	for (alarmIndex = 1;alarmIndex<ALARM_SYSTEM_QTY+1;alarmIndex++)
	{
		activeAlarmIndex = (actualAlarm+alarmIndex)%ALARM_SYSTEM_QTY;
		if (ALARM.system[activeAlarmIndex].isActive) 
      return activeAlarmIndex; 
	}
  return activeAlarmIndex; 
}

uint8_t UI_MedAlarmPrevActive(uint8_t actualAlarm)
{
	uint8_t alarmIndex, activeAlarmIndex;
	for (alarmIndex = 1;alarmIndex<ALARM_MEDICAL_QTY+1;alarmIndex++)
	{
		activeAlarmIndex = (ALARM_MEDICAL_QTY+actualAlarm-alarmIndex)%ALARM_MEDICAL_QTY;
		if (ALARM.medical[activeAlarmIndex].isActive) 
      return activeAlarmIndex; 
	}
  
  return activeAlarmIndex;
}

uint8_t UI_SysAlarmPrevActive(uint8_t actualAlarm)
{
	uint8_t alarmIndex, activeAlarmIndex;
	for (alarmIndex = 1;alarmIndex<ALARM_SYSTEM_QTY+1;alarmIndex++)
	{
		activeAlarmIndex = (ALARM_SYSTEM_QTY+actualAlarm-alarmIndex)%ALARM_SYSTEM_QTY;
		if (ALARM.system[activeAlarmIndex].isActive) 
      return activeAlarmIndex; 
	}

  return activeAlarmIndex;
}

uint8_t UI_MedAlarmActiveQty()
{
	uint8_t alarmIndex, activeAlarmQuantity = 0;
	for (alarmIndex = 0;alarmIndex<ALARM_MEDICAL_QTY;alarmIndex++)
	{
		if (ALARM.medical[alarmIndex].isActive) 
      activeAlarmQuantity++; 
	}
  return activeAlarmQuantity;
}

uint8_t UI_SysAlarmActiveQty()
{
	uint8_t alarmIndex, activeAlarmQuantity = 0;
	for (alarmIndex = 0;alarmIndex<ALARM_SYSTEM_QTY;alarmIndex++)
	{
		if (ALARM.system[alarmIndex].isActive) 
      activeAlarmQuantity++; 
	}
  return activeAlarmQuantity;
}

uint8_t UI_MedAlarmActiveIdx(uint8_t actualAlarm)
{
	uint8_t alarmIndex, activeAlarmIndex = 0;
	for (alarmIndex = 1;alarmIndex<=actualAlarm;alarmIndex++)
	{
		if (ALARM.medical[alarmIndex].isActive) 
      activeAlarmIndex++; 
	}
  return activeAlarmIndex;
}

uint8_t UI_SysAlarmActiveIdx(uint8_t actualAlarm)
{
	uint8_t alarmIndex, activeAlarmIndex = 0;
	for (alarmIndex = 1;alarmIndex<=actualAlarm;alarmIndex++)
	{
		if (ALARM.system[alarmIndex].isActive) 
      activeAlarmIndex++; 
	}
  return activeAlarmIndex;
}

void UI_MedAlarmActiveTurnOff(uint8_t actualAlarm)
{
  ALARM.medical[actualAlarm].isActive = false;  
  
  //uint8_t alarmIndex;
	//for (alarmIndex = 0;alarmIndex<ALARM_MEDICAL_QTY;alarmIndex++) 
  //{ 
  //  Serial.print(ALARM.medical[alarmIndex].isActive);
  //}
  //Serial.println();

  // ToDo> return bool and check the alarm is active
}

void UI_UpdateControlParam()
{
  static uint32_t timeOutResetDisplay;

  if(UI_Timer(TIMEOUT_UPDATE_CTRL_PARAM))
  {
    timeOutResetDisplay += TIMEOUT_UPDATE_CTRL_PARAM;

    showParam.breathsMinute     = CTRL.breathsMinute;
    showParam.dynamicCompliance = CTRL.dynamicCompliance;
    showParam.peakPressure      = CTRL.peakPressure;
    showParam.PEEP              = CTRL.PEEP;
    showParam.plateauPressure   = CTRL.plateauPressure;
    showParam.pressure          = CTRL.pressure;
    showParam.volume            = CTRL.volume;
    showParam.volumeMinute      = CTRL.volumeMinute;
    UI_Timer(0);
  }
  else if(timeOutResetDisplay >= TIMEOUT_UPDATE_CTRL_PARAM)
  {
    UI_DisplayClear();
    timeOutResetDisplay = 0;
  }
}

void UI_DisplayParameters(const char *param1, const char *param2, 
                          const char *val1, uint8_t pos1, const char *val2, uint8_t pos2)
{
  UI_DisplayMessage( 0,  0, param1);
  UI_DisplayMessage( 8,  0, param2);
  UI_DisplayMessage( pos1,  1, val1);
  UI_DisplayMessage( pos2,  1, val2);
}

void UI_DisplayMessage(uint8_t pos, uint8_t line, const char *message)
{
  lcd.setCursor(pos, line);
  lcd.print(message);
}

void UI_DisplayClear()
{
  lcd.begin(DISPLAY_COLUMNS, DISPLAY_ROWS);   // initialize the lcd 
  lcd.home ();
  lcd.clear();
}

bool UI_ButtonDebounce(uint8_t pin)
{
  static uint32_t debounceInitialMillis[ARDUINO_PIN_QTY]; 
  switch (debounceState[pin]) 
  {
    case BUTTON_RELEASED:
      switch (digitalRead(pin)) 
      {
        case LOW:
          if ( millis()-debounceInitialMillis[pin]> TIMEOUT_BUTTON_DEBOUNCE)
          {
            debounceState[pin] = BUTTON_PRESSED; 
            buttonState[pin] = true;
            debounceInitialMillis[pin] = millis();
            return true;
          }
          break;
        case HIGH:
          debounceInitialMillis[pin] = millis();
          break;
      }  
      break;
    case BUTTON_PRESSED:
      switch (digitalRead(pin)) 
      {
        case LOW:
          debounceInitialMillis[pin] = millis();
          break;
        case HIGH:
          if ( millis()-debounceInitialMillis[pin]>5 )
          {
            debounceState[pin] = BUTTON_RELEASED; 
            buttonState[pin] = false;
            debounceInitialMillis[pin] = millis();
          }
          break;
      }  
      break;      
    default:
        debounceState[pin] = BUTTON_RELEASED; 
        break;
  } 
  return false;
}

bool UI_Timer(uint32_t n)
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

bool UI_Timer2(uint32_t n)
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

bool UI_AlarmDisplayTimer(uint32_t n)
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

bool UI_AlarmBuzzerTimer(uint32_t n)
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

bool UI_AlarmMuteTimer(uint32_t n)
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