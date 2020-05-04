#include <Arduino.h>
#include "led_fw.h"
#include "user_interface.h"

// Variables
int ledState;            
unsigned long previousMillis ;        

void LedInit()
{
  pinMode(LED_FW_PIN, OUTPUT);
  ledState = LOW;

  previousMillis = millis();
}

void LedTick()
{
  unsigned long currentMillis = millis();

  if (UI_ActiveSystemAlarms())
  {
    digitalWrite(LED_FW_PIN, HIGH);
  }
  else
  {
    if (currentMillis - previousMillis >= LED_FW_PERIOD) 
    {
      previousMillis = currentMillis;

      if (ledState == LOW) {
        ledState = HIGH;
      } else {
        ledState = LOW;
      }

      digitalWrite(LED_FW_PIN, ledState);
    }
  }
}