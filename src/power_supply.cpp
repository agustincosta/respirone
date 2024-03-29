#include "power_supply.h"

POWER_t POWER;

void PowerSupply_Init()
{
  // Input signals
  pinMode(LOW_BATTERY_SIGNAL_PIN, INPUT);
  pinMode(MAINS_ON_SIGNAL_PIN, INPUT);

  // Output LEDs
  pinMode(LOW_BATTERY_LED_PIN, OUTPUT);
  pinMode(MAINS_ON_LED_PIN, OUTPUT);

  // Variables
  POWER.lowBattery = false; 
  POWER.mainsOn = false;   
}

void PowerSupply_Tasks()
{
  if (PowerSupply_Timer(POWER_SUPPLY_TIMEOUT))
  {
    PowerSupply_Timer(POWER_SUPPLY_START_TIMER);
    
    // Low battery
    POWER.lowBattery = digitalRead(LOW_BATTERY_SIGNAL_PIN);
    digitalWrite(LOW_BATTERY_LED_PIN, POWER.lowBattery?HIGH:LOW);  
  
    // Mains on
    POWER.mainsOn = digitalRead(MAINS_ON_SIGNAL_PIN);  
    digitalWrite(MAINS_ON_LED_PIN, POWER.mainsOn?HIGH:LOW);
  }
}

bool PowerSupply_Timer(uint32_t n)
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
