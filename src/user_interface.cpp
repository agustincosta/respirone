#include "user_interface.h"
#include <LiquidCrystal.h>

// Variables
// LCD
UI_states_e uiState;
//UNO
//LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
//MEGA
LiquidCrystal lcd(45, 44, 43, 42, 41, 40);

DebounceStates_t debounceState[ARDUINO_PIN_QTY];
bool buttonState[ARDUINO_PIN_QTY];

UI_t tempParam;
UI_t UI;

// User Interface

void UI_Init()
{
  // Debug
  Serial.begin(9600);

  // Display
  lcd.begin(16,2);               // initialize the lcd 
  lcd.home ();                   // go home
  UI_DisplayClear();
  UI_DisplayMessage(0,0,DISPLAY_PROJECT_NAME);
  UI_DisplayMessage(0,1,DISPLAY_LAUNCH_MENU);
  
  uiState = UI_WAITING_BUTTON;

  // Button
  pinMode(BUTTON_UP_PIN,    INPUT_PULLUP); 
  pinMode(BUTTON_DOWN_PIN,  INPUT_PULLUP); 
  pinMode(BUTTON_MENU_PIN,  INPUT_PULLUP); 
  pinMode(BUTTON_ENTER_PIN, INPUT_PULLUP); 
  pinMode(BUTTON_BACK_PIN,  INPUT_PULLUP); 

  // Timer
  UI_Timer(0); 

  // Default parameters
  tempParam.selectedMode = UI_AUTOMATIC_CONTROL;
  tempParam.tidalVolume = 500;
  tempParam.maxVolumeMinute = 14;
  tempParam.minVolumeMinute = 4;
  tempParam.breathsMinute = 14;
  tempParam.i_e = 2;
  tempParam.maxPressure = 30;
  tempParam.TrP = -4;
  tempParam.adjustedPressure = 20;
}

void UI_Task()
{
  char stringAux[16],
       stringAux2[16];

  stringAux[0] = 0;
  stringAux2[0] = 0;
  
  switch (uiState) 
  {
    case UI_WAITING_BUTTON:
      if(UI_ButtonDebounce(BUTTON_MENU_PIN))
      {
        UI_DisplayClear();
        UI_DisplayMessage(0,0,DISPLAY_SELECT_MODE);
        UI_DisplayMessage(0,1,DISPLAY_AUTO_MODE);
        uiState = UI_SET_MODE_AUTO;
        UI_Timer(0);
      }
      break;

    case UI_SET_MODE_AUTO:
        if(UI_ButtonDebounce(BUTTON_UP_PIN))  //pre
        {
          UI_DisplayMessage(0,1,DISPLAY_PRESSURE_MODE);
          uiState = UI_SET_MODE_PRESSURE;
        } 
        else if(UI_ButtonDebounce(BUTTON_DOWN_PIN)) //vol
        {
          UI_DisplayMessage(0,1,DISPLAY_VOLUME_MODE);
          uiState = UI_SET_MODE_VOLUME;
        }
        else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
        {
          UI.selectedMode = UI_AUTOMATIC_CONTROL;
          uiState = UI_SET_DEFAULT_PARAMETERS;
          UI_Timer(0);
        }
        else if(UI_ButtonDebounce(BUTTON_BACK_PIN))
        {
          UI_DisplayMessage(0,0,DISPLAY_PROJECT_NAME);
          UI_DisplayMessage(0,1,DISPLAY_LAUNCH_MENU);
          uiState = UI_WAITING_BUTTON;        
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
        UI_DisplayMessage(0,1,DISPLAY_PRESSURE_MODE);
        uiState = UI_SET_MODE_PRESSURE;
      } 
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN)) //vol
      {
        UI_DisplayMessage(0,1,DISPLAY_VOLUME_MODE);
        uiState = UI_SET_MODE_VOLUME;
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        UI.selectedMode = UI_AUTOMATIC_CONTROL;
        uiState = UI_SET_DEFAULT_PARAMETERS;
        UI_Timer(0);
      }
      else if(UI_ButtonDebounce(BUTTON_BACK_PIN))
      {
        UI_DisplayMessage(0,0,DISPLAY_PROJECT_NAME);
        UI_DisplayMessage(0,1,DISPLAY_LAUNCH_MENU);
        uiState = UI_WAITING_BUTTON;        
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
        UI_DisplayMessage(0,1,DISPLAY_PRESSURE_MODE);
        uiState = UI_SET_MODE_PRESSURE;
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        UI_DisplayMessage(0,1,DISPLAY_VOLUME_MODE);
        UI.selectedMode = UI_VOLUME_CONTROL;
        uiState = UI_DELAY_END_MODE;
        UI_Timer(0);
      }
      else if(UI_ButtonDebounce(BUTTON_BACK_PIN))
      {
        UI_DisplayMessage(0,0,DISPLAY_PROJECT_NAME);
        UI_DisplayMessage(0,1,DISPLAY_LAUNCH_MENU);
        uiState = UI_WAITING_BUTTON;        
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
        UI_DisplayMessage(0,1,DISPLAY_PRESSURE_MODE);
        uiState = UI_SET_MODE_PRESSURE;
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        UI_DisplayMessage(0,1,DISPLAY_VOLUME_MODE);
        UI.selectedMode = UI_VOLUME_CONTROL;
        uiState = UI_DELAY_END_MODE;
        UI_Timer(0);
      }
      else if(UI_ButtonDebounce(BUTTON_BACK_PIN))
      {
        UI_DisplayMessage(0,0,DISPLAY_PROJECT_NAME);
        UI_DisplayMessage(0,1,DISPLAY_LAUNCH_MENU);
        uiState = UI_WAITING_BUTTON;        
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
        UI.selectedMode = UI_PRESSURE_CONTROL;
        uiState = UI_DELAY_END_MODE;
        UI_Timer(0);
      }
      else if(UI_ButtonDebounce(BUTTON_BACK_PIN))
      {
        UI_DisplayMessage(0,0,DISPLAY_PROJECT_NAME);
        UI_DisplayMessage(0,1,DISPLAY_LAUNCH_MENU);
        uiState = UI_WAITING_BUTTON;        
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
        UI.selectedMode = UI_PRESSURE_CONTROL;
        uiState = UI_DELAY_END_MODE;
        UI_Timer(0);
      }
      else if(UI_ButtonDebounce(BUTTON_BACK_PIN))
      {
        UI_DisplayMessage(0,0,DISPLAY_PROJECT_NAME);
        UI_DisplayMessage(0,1,DISPLAY_LAUNCH_MENU);
        uiState = UI_WAITING_BUTTON;        
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
        if(UI.selectedMode == UI_PRESSURE_CONTROL)
        {
          UI_DisplayClear();
          UI_DisplayMessage(0,0,DISPLAY_ADJUSTED_PRESSURE);

          itoa(tempParam.adjustedPressure,stringAux,10);
          strcat(stringAux, "cm.H2O");
          UI_DisplayMessage(0,1,stringAux);

          uiState = UI_SET_ADJUSTED_PRESSURE;
          UI_Timer(0);
        }
        else if(UI.selectedMode == UI_VOLUME_CONTROL)
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

    case UI_SET_ADJUSTED_PRESSURE:
      if(UI_ButtonDebounce(BUTTON_UP_PIN))  
      {
        (tempParam.adjustedPressure < 40) ? tempParam.adjustedPressure += 1 : tempParam.adjustedPressure = 40;
        itoa(tempParam.adjustedPressure,stringAux,10);
        strcat(stringAux, "cm.H2O");
        UI_DisplayMessage(0,1,stringAux);

      } 
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN)) 
      {
        (tempParam.adjustedPressure > 5)? tempParam.adjustedPressure -= 1 : tempParam.adjustedPressure = 1;
        itoa(tempParam.adjustedPressure,stringAux,10);
        strcat(stringAux, "cm.H2O");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        itoa(tempParam.adjustedPressure,stringAux,10);
        strcat(stringAux, "cm.H2O");
        UI_DisplayMessage(0,1,stringAux);
        UI.adjustedPressure = tempParam.adjustedPressure;
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
        (tempParam.adjustedPressure < 40) ? tempParam.tidalVolume += 1 : tempParam.tidalVolume = 40;
        itoa(tempParam.adjustedPressure,stringAux,10);
        strcat(stringAux, "cm.H2O");
        UI_DisplayMessage(0,1,stringAux);
      } 
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN)) 
      {
        (tempParam.tidalVolume > 5) ? tempParam.tidalVolume -= 1 : tempParam.tidalVolume = 5;
        itoa(tempParam.adjustedPressure,stringAux,10);
        strcat(stringAux, "cm.H2O");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        itoa(tempParam.adjustedPressure,stringAux,10);
        strcat(stringAux, "cm.H2O");
        UI_DisplayMessage(0,1,stringAux);
        UI.adjustedPressure = tempParam.adjustedPressure;
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

    case UI_SET_TIDAL_VOLUME:
      if(UI_ButtonDebounce(BUTTON_UP_PIN))  
      {
        (tempParam.tidalVolume < 800) ? tempParam.tidalVolume += 50 : tempParam.tidalVolume = 800;
        itoa(tempParam.tidalVolume,stringAux,10);
        strcat(stringAux, "ml");
        UI_DisplayMessage(0,1,stringAux);

      } 
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN)) 
      {
        (tempParam.tidalVolume > 300)? tempParam.tidalVolume -= 50 : tempParam.tidalVolume = 300;
        itoa(tempParam.tidalVolume,stringAux,10);
        strcat(stringAux, "ml");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        itoa(tempParam.tidalVolume,stringAux,10);
        strcat(stringAux, "ml");
        UI_DisplayMessage(0,1,stringAux);
        UI.tidalVolume = tempParam.tidalVolume;
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
        (tempParam.tidalVolume < 800) ? tempParam.tidalVolume += 50 : tempParam.tidalVolume = 800;
        itoa(tempParam.tidalVolume,stringAux,10);
        strcat(stringAux, "ml");
        UI_DisplayMessage(0,1,stringAux);
      } 
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN)) 
      {
        (tempParam.tidalVolume > 300) ? tempParam.tidalVolume -= 50 : tempParam.tidalVolume = 300;
        itoa(tempParam.tidalVolume,stringAux,10);
        strcat(stringAux, "ml");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        itoa(tempParam.tidalVolume,stringAux,10);
        strcat(stringAux, "ml");
        UI_DisplayMessage(0,1,stringAux);
        UI.tidalVolume = tempParam.tidalVolume;
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

    case UI_SET_VOLUME_MINUTE_M:
      if(UI_ButtonDebounce(BUTTON_UP_PIN)) 
      {
        (tempParam.maxVolumeMinute < 20) ? tempParam.maxVolumeMinute += 1 : tempParam.maxVolumeMinute = 20;
        itoa(tempParam.maxVolumeMinute,stringAux,10);
        strcat(stringAux, "L/m");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN))
      {
        (tempParam.maxVolumeMinute > 12)? tempParam.maxVolumeMinute -= 1 : tempParam.maxVolumeMinute = 12;
        itoa(tempParam.maxVolumeMinute,stringAux,10);
        strcat(stringAux, "L/m");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        itoa(tempParam.maxVolumeMinute,stringAux,10);
        strcat(stringAux, "L/m");
        UI_DisplayMessage(0,1,stringAux);
        UI.maxVolumeMinute = tempParam.maxVolumeMinute;
        uiState = UI_DELAY_END_VOLUME_MINUTE_M;
        UI_Timer(0);
      }
      else if(UI_ButtonDebounce(BUTTON_BACK_PIN))
      {
        UI_DisplayClear();
        UI_DisplayMessage(0,0,DISPLAY_TIDAL_VOLUME);
        
        itoa(tempParam.tidalVolume,stringAux,10);
        strcat(stringAux, "ml");
        UI_DisplayMessage(0,1,stringAux);

        uiState = UI_SET_TIDAL_VOLUME;
        UI_Timer(0);         
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
        (tempParam.maxVolumeMinute < 20) ? tempParam.maxVolumeMinute += 1 : tempParam.maxVolumeMinute = 20;
        itoa(tempParam.maxVolumeMinute,stringAux,10);
        strcat(stringAux, "L/m");
        UI_DisplayMessage(0,1,stringAux);
      } 
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN)) 
      {
        (tempParam.maxVolumeMinute > 12)? tempParam.maxVolumeMinute -= 1 : tempParam.maxVolumeMinute = 12;
        itoa(tempParam.maxVolumeMinute,stringAux,10);
        strcat(stringAux, "L/m");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        itoa(tempParam.maxVolumeMinute,stringAux,10);
        strcat(stringAux, "L/m");
        UI_DisplayMessage(0,1,stringAux);
        UI.maxVolumeMinute = tempParam.maxVolumeMinute;
        uiState = UI_DELAY_END_VOLUME_MINUTE_M;
        UI_Timer(0);
      }
      else if(UI_ButtonDebounce(BUTTON_BACK_PIN))
      {
        UI_DisplayClear();
        UI_DisplayMessage(0,0,DISPLAY_TIDAL_VOLUME);

        itoa(tempParam.tidalVolume,stringAux,10);
        strcat(stringAux, "ml");
        UI_DisplayMessage(0,1,stringAux);

        uiState = UI_SET_TIDAL_VOLUME;
        UI_Timer(0);         
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

          itoa(tempParam.maxVolumeMinute,stringAux,10);
          strcat(stringAux, "L/m");
          UI_DisplayMessage(0,1,stringAux);

          uiState = UI_SET_VOLUME_MINUTE_m;
          UI_Timer(0);
      }    
      break;

    case UI_SET_VOLUME_MINUTE_m:
      if(UI_ButtonDebounce(BUTTON_UP_PIN)) 
      {
        (tempParam.maxVolumeMinute < 5) ? tempParam.maxVolumeMinute += 1 : tempParam.maxVolumeMinute = 5;
        itoa(tempParam.maxVolumeMinute,stringAux,10);
        strcat(stringAux, "L/m");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN))
      {
        (tempParam.maxVolumeMinute > 1)? tempParam.maxVolumeMinute -= 1 : tempParam.maxVolumeMinute = 1;
        itoa(tempParam.maxVolumeMinute,stringAux,10);
        strcat(stringAux, "L/m");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        itoa(tempParam.maxVolumeMinute,stringAux,10);
        strcat(stringAux, "L/m");
        UI_DisplayMessage(0,1,stringAux);
        UI.maxVolumeMinute = tempParam.maxVolumeMinute;
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
        (tempParam.maxVolumeMinute < 5) ? tempParam.maxVolumeMinute += 1 : tempParam.maxVolumeMinute = 5;
        itoa(tempParam.maxVolumeMinute,stringAux,10);
        strcat(stringAux, "L/m");
        UI_DisplayMessage(0,1,stringAux);
      } 
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN)) 
      {
        (tempParam.maxVolumeMinute > 1)? tempParam.maxVolumeMinute -= 1 : tempParam.maxVolumeMinute = 1;
        itoa(tempParam.maxVolumeMinute,stringAux,10);
        strcat(stringAux, "L/m");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        itoa(tempParam.maxVolumeMinute,stringAux,10);
        strcat(stringAux, "L/m");
        UI_DisplayMessage(0,1,stringAux);
        UI.maxVolumeMinute = tempParam.maxVolumeMinute;
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
        itoa(tempParam.maxVolumeMinute,stringAux,10);
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

    case UI_SET_RPM:
      if(UI_ButtonDebounce(BUTTON_UP_PIN)) 
      {
        (tempParam.breathsMinute < 30) ? tempParam.breathsMinute += 1 : tempParam.breathsMinute = 30;
        itoa(tempParam.breathsMinute,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN))
      {
        (tempParam.breathsMinute > 5)? tempParam.breathsMinute -= 1 : tempParam.breathsMinute = 5;
        itoa(tempParam.breathsMinute,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        itoa(tempParam.breathsMinute,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
        UI.breathsMinute = tempParam.breathsMinute;
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
        (tempParam.breathsMinute < 30) ? tempParam.breathsMinute += 1 : tempParam.breathsMinute = 30;
        itoa(tempParam.breathsMinute,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
      } 
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN)) 
      {
        (tempParam.breathsMinute > 5)? tempParam.breathsMinute -= 1 : tempParam.breathsMinute = 5;
        itoa(tempParam.breathsMinute,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        itoa(tempParam.breathsMinute,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
        UI.breathsMinute = tempParam.breathsMinute;
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
        UI_DisplayMessage(0,0,DISPLAY_I_E);

        strcat(stringAux,"1:");
        itoa(tempParam.i_e,stringAux2,10);
        strcat(stringAux,stringAux2);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);

        uiState = UI_SET_I_E;
        UI_Timer(0);
      }    
      break;

    case UI_SET_I_E:
      if(UI_ButtonDebounce(BUTTON_UP_PIN)) 
      {
        (tempParam.i_e < 4) ? tempParam.i_e += 1 : tempParam.i_e = 4;
        strcat(stringAux,"1:");
        itoa(tempParam.i_e,stringAux2,10);
        strcat(stringAux,stringAux2);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN))
      {
        (tempParam.i_e > 1)? tempParam.i_e -= 1 : tempParam.i_e = 1;
        strcat(stringAux,"1:");
        itoa(tempParam.i_e,stringAux2,10);
        strcat(stringAux,stringAux2);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        strcat(stringAux,"1:");
        itoa(tempParam.i_e,stringAux2,10);
        strcat(stringAux,stringAux2);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
        UI.i_e = tempParam.i_e;
        uiState = UI_DELAY_END_I_E;
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
        uiState = UI_BLINK_I_E;
        UI_DisplayMessage(0,1,DISPLAY_EMPTY_LINE);
        UI_Timer(0);
      }        
      break;

    case UI_BLINK_I_E:
      if(UI_ButtonDebounce(BUTTON_UP_PIN))  
      {
        (tempParam.i_e < 4) ? tempParam.i_e += 1 : tempParam.i_e = 4;
        strcat(stringAux,"1:");
        itoa(tempParam.i_e,stringAux2,10);
        strcat(stringAux,stringAux2);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
      } 
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN)) 
      {
        (tempParam.i_e > 1)? tempParam.i_e -= 1 : tempParam.i_e = 1;
        strcat(stringAux,"1:");
        itoa(tempParam.i_e,stringAux2,10);
        strcat(stringAux,stringAux2);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        strcat(stringAux,"1:");
        itoa(tempParam.i_e,stringAux2,10);
        strcat(stringAux,stringAux2);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
        UI.i_e = tempParam.i_e;
        uiState = UI_DELAY_END_I_E;
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
        strcat(stringAux,"1:");
        itoa(tempParam.i_e,stringAux2,10);
        strcat(stringAux,stringAux2);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);

        uiState = UI_SET_I_E;
        UI_Timer(0);
      }       
      break;

    case UI_DELAY_END_I_E:
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

    case UI_SET_MAX_PRESSURE:
      if(UI_ButtonDebounce(BUTTON_UP_PIN)) 
      {
        (tempParam.maxPressure < 55) ? tempParam.maxPressure += 1 : tempParam.maxPressure = 55;
        itoa(tempParam.maxPressure,stringAux,10);
        strcat(stringAux, "cm.H2O");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN))
      {
        (tempParam.maxPressure > 35)? tempParam.maxPressure -= 1 : tempParam.maxPressure = 35;
        itoa(tempParam.maxPressure,stringAux,10);
        strcat(stringAux, "cm.H2O");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        itoa(tempParam.maxPressure,stringAux,10);
        strcat(stringAux, "cm.H2O");
        UI_DisplayMessage(0,1,stringAux);
        UI.maxPressure = tempParam.maxPressure;
        uiState = UI_DELAY_END_MAX_PRESSURE;
        UI_Timer(0);
      }
      else if(UI_ButtonDebounce(BUTTON_BACK_PIN))
      {
        UI_DisplayClear();
        UI_DisplayMessage(0,0,DISPLAY_I_E);
        
        strcat(stringAux,"1:");
        itoa(tempParam.i_e,stringAux2,10);
        strcat(stringAux,stringAux2);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);

        uiState = UI_SET_I_E;
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
        (tempParam.maxPressure < 55) ? tempParam.maxPressure += 1 : tempParam.maxPressure = 55;
        itoa(tempParam.maxPressure,stringAux,10);
        strcat(stringAux, "cm.H2O");
        UI_DisplayMessage(0,1,stringAux);
      } 
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN)) 
      {
        (tempParam.maxPressure > 35)? tempParam.maxPressure -= 1 : tempParam.maxPressure = 35;
        itoa(tempParam.maxPressure,stringAux,10);
        strcat(stringAux, "cm.H2O");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        itoa(tempParam.maxPressure,stringAux,10);
        strcat(stringAux, "cm.H2O");
        UI_DisplayMessage(0,1,stringAux);
        UI.maxPressure = tempParam.maxPressure;
        uiState = UI_DELAY_END_MAX_PRESSURE;
        UI_Timer(0);
      }
      else if(UI_ButtonDebounce(BUTTON_BACK_PIN))
      {
        UI_DisplayClear();
        UI_DisplayMessage(0,0,DISPLAY_I_E);
        
        strcat(stringAux,"1:");
        itoa(tempParam.i_e,stringAux2,10);
        strcat(stringAux,stringAux2);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);

        uiState = UI_SET_I_E;
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
          UI_DisplayMessage(0,0,DISPLAY_TRP);

          itoa(tempParam.TrP,stringAux,10);
          strcat(stringAux, " ");
          UI_DisplayMessage(0,1,stringAux);

          uiState = UI_SET_TRP;
          UI_Timer(0);
      }    
      break;

    case UI_SET_TRP:
      if(UI_ButtonDebounce(BUTTON_UP_PIN)) 
      {
        (tempParam.TrP < 0) ? tempParam.TrP += 1 : tempParam.TrP = 0;
        itoa(tempParam.TrP,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN))
      {
        (tempParam.TrP > -16)? tempParam.TrP -= 1 : tempParam.TrP = -16;
        itoa(tempParam.TrP,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        itoa(tempParam.TrP,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
        UI.TrP = tempParam.TrP;
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
        uiState = UI_BLINK_TRP;
        UI_DisplayMessage(0,1,DISPLAY_EMPTY_LINE);
        UI_Timer(0);
      }        
      break;

    case UI_BLINK_TRP:
      if(UI_ButtonDebounce(BUTTON_UP_PIN))  
      {
        (tempParam.TrP < 0) ? tempParam.TrP += 1 : tempParam.TrP = 0;
        itoa(tempParam.TrP,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
      } 
      else if(UI_ButtonDebounce(BUTTON_DOWN_PIN)) 
      {
        (tempParam.TrP > -16)? tempParam.TrP -= 1 : tempParam.TrP = -16;
        itoa(tempParam.TrP,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
      }
      else if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        itoa(tempParam.TrP,stringAux,10);
        strcat(stringAux, " ");
        UI_DisplayMessage(0,1,stringAux);
        UI.TrP = tempParam.TrP;
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
          UI_DisplayMessage(0,1,"SI:Enter,NO:Back");
          UI_DisplayMessage(0,1,stringAux);

          uiState = UI_CONFIRM_CONFIG_PARAMETERS;
      }    
      break;

    case UI_SET_DEFAULT_PARAMETERS:
      UI.i_e            = tempParam.i_e;
      UI.maxPressure    = tempParam.maxPressure;
      UI.breathsMinute  = tempParam.breathsMinute;
      UI.tidalVolume    = tempParam.tidalVolume;
      UI.TrP            = tempParam.TrP;
      UI.maxVolumeMinute = tempParam.maxVolumeMinute;
      UI.maxVolumeMinute = tempParam.maxVolumeMinute;
      if(UI_Timer(TIMEOUT_SHOW_SELECTED_PARAM))
      {
        UI_DisplayClear();
        (UI.selectedMode == UI_SET_MODE_AUTO) ?
          UI_DisplayMessage(0,0,DISPLAY_CONFIRMATION) : UI_DisplayMessage(0,0,DISPLAY_AUTO_CONFIRMATION);
        UI_DisplayMessage(0,1,"SI:Enter,NO:Back");
        UI_DisplayMessage(0,1,stringAux);
        uiState = UI_CONFIRM_CONFIG_PARAMETERS;
      } 
      break;

    case UI_CONFIRM_CONFIG_PARAMETERS:
      if(UI_ButtonDebounce(BUTTON_ENTER_PIN))
      {
        UI.setUpComplete = true;
        uiState = UI_SHOW_PARAMETERS;
        UI_DisplayClear();
        UI_DisplayMessage(0,0,"TO DO:");
        UI_DisplayMessage(0,1,"SHOW PARAMETERS");
        UI_Timer(0);

        // Debug
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
        Serial.println(UI.maxVolumeMinute);
        Serial.print("RPM:                ");
        Serial.println(UI.breathsMinute);
        Serial.print("I:E:                ");
        Serial.println(UI.i_e);
        Serial.print("Presión Máx:        ");
        Serial.println(UI.maxPressure);
        Serial.print("TrP:                ");
        Serial.println(UI.TrP);
      }
      else if(UI_ButtonDebounce(BUTTON_BACK_PIN))
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

    case UI_SHOW_PARAMETERS:
      if(UI_ButtonDebounce(BUTTON_MENU_PIN))
      {
        uiState = UI_RESTART_CONFIG;
        UI_Timer(0);
      }
      else
      {
      // execute state machine with real time parameters
      UI_ShowParametersTask();
      }
      break;

    case UI_RESTART_CONFIG:
      UI_ButtonDebounce(BUTTON_MENU_PIN);

      if((buttonState[BUTTON_MENU_PIN]) && (UI_Timer(TIMEOUT_RESTART_CONFIG)))
      {
        UI_Init();
      }
      else if(!buttonState[BUTTON_MENU_PIN])
      {
        uiState = UI_SHOW_PARAMETERS;
      }
      break;

    default:
      uiState = UI_WAITING_BUTTON;
      break;
  }
}

void UI_ShowParametersTask()
{

}

void UI_DisplayMessage(uint8_t pos, uint8_t line, const char *message)
{
  lcd.setCursor(pos, line);
  lcd.print(message);
}

void UI_DisplayClear()
{
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
          if ( millis()-debounceInitialMillis[pin]>5 )
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

void UI_SetAlarm(uint8_t alarm)
{

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

