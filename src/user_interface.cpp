#include "user_interface.h"
#include <LiquidCrystal.h>

// Variables
// LCD
UI_states_e uiState;
//UNO
//LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
//MEGA
LiquidCrystal lcd(45, 44, 43, 42, 41, 40);

DebounceStates debounceState;

// User Interface

void UserInterface_Init()
{
  // Display
  lcd.begin(16,2);               // initialize the lcd 
  lcd.home ();                   // go home
  UI_DisplayClear();
  UI_DisplayMessage(0,0,PROJECT_NAME);
  UI_DisplayMessage(0,1,LAUNCH_MENU);
  
  uiState = UI_WAITING_BUTTON;

  // Button
  pinMode(BUTTON_AUTO_PIN,   INPUT_PULLUP); 
  pinMode(BUTTON_MENU_PIN,   INPUT_PULLUP); 
  pinMode(BUTTON_RIGHT_PIN,  INPUT_PULLUP); 
  pinMode(BUTTON_LEFT_PIN,   INPUT_PULLUP); 
  pinMode(BUTTON_SELECT_PIN, INPUT_PULLUP); 

  // Timer
  UI_Timer(0); 
}

void UserInterface_Task()
{

  switch (uiState) 
  {
    case UI_WAITING_BUTTON:
      if(UI_ButtonDebounce(BUTTON_MENU_PIN))
      {
        UI_DisplayClear();
        UI_DisplayMessage(0,0,SELECT_MODE);
        UI_DisplayMessage(0,1,VOLUME_MODE);
        uiState = UI_SET_MODE_VOLUME;
        UI_Timer(0);
      }
      else if(UI_ButtonDebounce(BUTTON_AUTO_PIN))
      {
        UI_DisplayMessage(0,0,SET_DEFAULT);
        uiState = UI_SET_DEFAULT_PARAMETERS; 
      }
      break;

    case UI_SET_MODE_VOLUME:
      if((UI_ButtonDebounce(BUTTON_LEFT_PIN)) || (UI_ButtonDebounce(BUTTON_RIGHT_PIN)))
      {
        UI_DisplayMessage(0,1,EMPTY_LINE);
        UI_DisplayMessage(0,1,PRESSION_MODE);
        uiState = UI_SET_MODE_PRESSION;
      }
      else if(UI_ButtonDebounce(BUTTON_SELECT_PIN))
      {
        //LOAD VARIABLE "MODE"
        //next config step
      }
      else if(UI_Timer(TIMEOUT_BLINK))
      {
        uiState = UI_BLINK_VOLUME;
        UI_DisplayMessage(0,1,EMPTY_LINE);
        UI_Timer(0);
      }
      break;

    case UI_BLINK_VOLUME:
      if((UI_ButtonDebounce(BUTTON_LEFT_PIN)) || (UI_ButtonDebounce(BUTTON_RIGHT_PIN)))
      {
        UI_DisplayMessage(0,1,EMPTY_LINE);
        UI_DisplayMessage(0,1,PRESSION_MODE);
        uiState = UI_SET_MODE_PRESSION;
      }
      else if(UI_ButtonDebounce(BUTTON_SELECT_PIN))
      {
        //LOAD VARIABLE "MODE"
      }
      else if(UI_Timer(TIMEOUT_BLINK/2))
      {
        UI_DisplayMessage(0,1,VOLUME_MODE);
        uiState = UI_SET_MODE_VOLUME; 
        UI_Timer(0);
      }
      break;

    case UI_SET_MODE_PRESSION:

      if((UI_ButtonDebounce(BUTTON_LEFT_PIN)) || (UI_ButtonDebounce(BUTTON_RIGHT_PIN)))
      {
        UI_DisplayMessage(0,1,EMPTY_LINE);
        UI_DisplayMessage(0,1,VOLUME_MODE);
        uiState = UI_SET_MODE_VOLUME;
      }
      else if(UI_ButtonDebounce(BUTTON_SELECT_PIN))
      {
        //LOAD VARIABLE "MODE"
      }
      else if(UI_Timer(TIMEOUT_BLINK))
      {
        uiState = UI_BLINK_PRESSION;
        UI_DisplayMessage(0,1,EMPTY_LINE);
        UI_Timer(0);
      }
      break;

    case UI_BLINK_PRESSION:
      if((UI_ButtonDebounce(BUTTON_LEFT_PIN)) || (UI_ButtonDebounce(BUTTON_RIGHT_PIN)))
      {
        UI_DisplayMessage(0,1,EMPTY_LINE);
        UI_DisplayMessage(0,1,VOLUME_MODE);
        uiState = UI_SET_MODE_VOLUME;
      }
      else if(UI_ButtonDebounce(BUTTON_SELECT_PIN))
      {
        //LOAD VARIABLE "MODE"
      }
      else if(UI_Timer(TIMEOUT_BLINK/2))
      {
        UI_DisplayMessage(0,1,EMPTY_LINE);
        UI_DisplayMessage(0,1,PRESSION_MODE);
        uiState = UI_SET_MODE_PRESSION; 
        UI_Timer(0);
      }
      break;



    case UI_SET_DEFAULT_PARAMETERS:
      //TO DO
      break;

    case UI_START:
      // // actual state tasks executed always
    
      // // next state
      // uiState = UI_WAIT_BUTTON; 

      // // next state tasks executed once
      // lcd.clear();
      // lcd.setCursor(0, 0);
      // lcd.print("Press any button");
      // UI_Timer(0);
      break;
      
    // case UI_WAIT_BUTTON:
    //   if (UI_ButtonDebounce(BUTTON_OK_PIN))
    //   {
    //     uiState = UI_DISPLAY_SUCCESS;
		
		//     lcd.clear();     
    //     lcd.setCursor(0, 0);
    //     lcd.print("Success!");
    //     UI_Timer(0);
    //   }
    //   else if (UI_Timer(5000))
    //   {
    //     uiState = UI_DISPLAY_ERROR;
        
    //     lcd.clear();
    //     lcd.setCursor(0, 0);
    //     lcd.print("Error!");
    //     UI_Timer(0);     
    //   }
    //   break;

    // case UI_DISPLAY_SUCCESS:
    //   if(UI_Timer(2000))
    //   {
    //     uiState = UI_START;
    //   }
    //   break;
      
    // case UI_DISPLAY_ERROR:
    //   if(UI_Timer(2000))
    //   {
    //     uiState = UI_START;
    //   }
    //   break;
  
    default:
      uiState = UI_START;
      break;
  }
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
  static uint8_t debounceInitialMillis[53]; 

  switch (debounceState) 
  {
    case WAIT_FOR_BUTTON:
      if(digitalRead(pin)==LOW) 
      {
        if ( (uint8_t)millis()-debounceInitialMillis[pin]>50)
        {
          debounceState = PRESSED_BUTTON; 
          return true;
        }
      }
      else
      {
        debounceInitialMillis[pin] = (uint8_t)millis();
      }   
      break;
      
    case PRESSED_BUTTON:
      if (digitalRead(pin)==HIGH) 
      {
        debounceState = WAIT_FOR_BUTTON; 
      }
      break; 

    default:
        debounceState = WAIT_FOR_BUTTON; 
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

