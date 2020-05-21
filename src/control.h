/**
 * @file control.h
 * @brief Control module
 * @version 1.0
 */
#ifndef CONTROL_H
#define CONTROL_H

#include "Arduino.h"

#define MINUTE_MS 60000

/**
 * @brief 
 */
typedef struct control
{
    float breathsMinute;      // Measured value of breath/minute

    float pressure;           // Actual pressure value (cmH2O)
    float peakPressure;       // Peak pressure in the last cycle (cmH2O)
    float plateauPressure;    // Plateau pressure in the last cycle (cmH2O)
    float PEEP;               // Measured value of PEEP in the last cycle (cmH2O)

    float flow;               // Actual flow value (mL)
    float measuredVolume;     // Measured volume (flow) (mL)
    
    // ToDo change volume to estimatedVolume
    float volume;             // Actual volume value (mL)
    float volumeMinute;       // Volume/minute in the last minute (L/m)

    float dynamicCompliance;  // Pulmonary compliance (V/P) in each cycle 

    float currentConsumption; // Motor consumption (A)
}
CTRL_t;

extern CTRL_t CTRL;

/**
 *  @brief 
 * 
 */
void Control_Init();

/**
 *  @brief  
 * 
 */
void Control_Task();

#endif