/**
 * @file power_supply.h
 * @brief 
 * @version 0.1
 */
#ifndef POWER_SUPPLY_H
#define POWER_SUPPLY_H

#include "Arduino.h"

/**
 * @section Defines
 */
#define LOW_BATTERY_SIGNAL_PIN                 A3
#define MAINS_ON_SIGNAL_PIN                    A5

#define LOW_BATTERY_LED_PIN                    8
#define MAINS_ON_LED_PIN                       9


#define POWER_SUPPLY_START_TIMER               0
#define POWER_SUPPLY_TIMEOUT                   1000

/**
 * @section Data types
 */
typedef struct
{
  bool lowBattery; 
  bool mainsOn;            
}
POWER_t;

extern POWER_t POWER;

/**
 *  @brief 
 */
void PowerSupply_Init();

/**
 * @brief 
 */
void PowerSupply_Tasks();

/**
 * 
 */
bool PowerSupply_Timer(uint32_t n);

#endif
