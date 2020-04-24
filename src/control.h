/**
 * @file control.h
 * @brief Control module
 * @version 0.1
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
    uint8_t breathsMinute;    // Measured value of breath/minute

    int16_t pressure;           // Actual pressure value
    int16_t peakPressure;      // Peak pressure in the last cycle
    int16_t plateauPressure;   // Plateau pressure in the last cycle
    int16_t PEEP;              // Measured value of PEEP in the last cycle

    int16_t volume;            // Actual volume value
    int16_t volumeMinute;   // Volume/minute in the last minute
}
CTRL_t;

extern CTRL_t CTRL;

/**
 * @brief Ventilation modes
 */
typedef enum 
{
    VOLUME_CONTROL,
    PRESSURE_CONTROL,
    CONTROL_IDLE
}
Control_Modes_e; 

/**
 * @brief FSM states for Volume Control
 */
typedef enum 
{
    CTRL_VOLUME_INIT,
    CTRL_VOLUME_INSPIRATION_SETUP,
    CTRL_VOLUME_INSPIRATION_MONITORING,
    CTRL_VOLUME_EXPIRATION_SETUP,
    CTRL_VOLUME_EXPIRATION_IDLE    
}
Control_VolumeStates_e; 

/**
 * @brief FSM states for Pressure Control
 */
typedef enum 
{
    CTRL_PRESSURE_INIT,
    CTRL_PRESSURE_INSPIRATION_SETUP,
    CTRL_PRESSURE_INSPIRATION_MONITORING,
    CTRL_PRESSURE_EXPIRATION_SETUP,
    CTRL_PRESSURE_EXPIRATION_IDLE    
}
Control_PressureStates_e; 

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

/**
 * @brief 
 * 
 */
bool CTRL_Timer(uint32_t n);

#endif