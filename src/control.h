/**
 * @file control.h
 * @brief Control module
 * @version 0.1
 */
#ifndef CONTROL_H
#define CONTROL_H

#include "Arduino.h"

typedef enum 
{
    VOLUME_CONTROL,
    PRESSURE_CONTROL,
    CONTROL_IDLE
}
Control_Modes_e; 

typedef enum 
{
    VENTILATOR_SETUP,
    VENTILATOR_INSPIRATION,
    VENTILATOR_EXPIRATION
}
Control_VentilatorStates_e; 

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

/**
 * @brief 
 * 
 */
void Control_VolumeModeTask();

/**
 * @brief 
 * 
 */
void Control_PressureModeTask();

#endif