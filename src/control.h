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

    float pressure;           // Actual pressure value
    float peakPressure;       // Peak pressure in the last cycle
    float plateauPressure;    // Plateau pressure in the last cycle
    float PEEP;               // Measured value of PEEP in the last cycle

    float volume;             // Actual volume value
    float volumeMinute;       // Volume/minute in the last minute
    float dynamicCompliance;    // Pulmonary compliance (V/P) in each cycle
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