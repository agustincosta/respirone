#include "user_interface.h"
#include <LiquidCrystal.h>

// Variables

UI_t UI;

// LCD
UI_states_e uiState;
//UNO
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
//MEGA
//LiquidCrystal lcd(45, 44, 43, 42, 41, 40);

DebounceStates debounceState;

// User Interface

void UserInterface_Init()
{
  // Display
  lcd.begin(16,2);               // initialize the lcd 
  lcd.home ();                   // go home
  lcd.print("RESPIRONE");  
  lcd.setCursor ( 0, 1 );        // go to the next line
  lcd.print ("Demo Display ");     
  
  uiState = UI_INIT_MESSAGE;

  // Button
  pinMode(BUTTON_OK_PIN, INPUT_PULLUP); 

  // Timer
  UI_Timer(0); 
}

void UserInterface_Task()
{

  switch (uiState) 
  {
    case UI_INIT_MESSAGE:
    // actual state tasks executed always
      if(UI_Timer(INIT_MESSAGE_TIMEOUT))
        // next state
        uiState = UI_START;
      break;

    case UI_START:
      // actual state tasks executed always
    
      // next state
      uiState = UI_WAIT_BUTTON; 

      // next state tasks executed once
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Press any button");
      UI_Timer(0);
      break;
      
    case UI_WAIT_BUTTON:
      if (UI_ButtonDebounce(BUTTON_OK_PIN))
      {
        uiState = UI_DISPLAY_SUCCESS;
		
		    lcd.clear();     
        lcd.setCursor(0, 0);
        lcd.print("Success!");
        UI_Timer(0);
      }
      else if (UI_Timer(5000))
      {
        uiState = UI_DISPLAY_ERROR;
        
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Error!");
        UI_Timer(0);     
      }
      break;

    case UI_DISPLAY_SUCCESS:
      if(UI_Timer(2000))
      {
        uiState = UI_START;
      }
      break;
      
    case UI_DISPLAY_ERROR:
      if(UI_Timer(2000))
      {
        uiState = UI_START;
      }
      break;
  
    default:
      uiState = UI_START;
      break;
  }
}

bool UI_ButtonDebounce(uint8_t pin)
{
  static uint8_t debounceInitialMillis[13]; 

  switch (debounceState) 
  {
    case WAIT_FOR_BUTTON:
      if (digitalRead(pin)==LOW) 
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

