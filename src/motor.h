/**
 * @file motor.h
 * @brief 
 * @version 0.1
 */
#ifndef MOTOR_H
#define MOTOR_H

#include "Arduino.h"

/**
 * @brief 
 */
typedef struct
{
    bool limitSwitch;           // Switch status
    uint8_t breathPerMinute;    // Measured value of breath/minute
    uint16_t volumePerMinute;   // Volume/minute in the last minute
}
MOTOR_t;

/**
 * @brief FSM states for the motor
 */
typedef enum 
{
    MOTOR_IDLE,
    MOTOR_RETURN_HOME_POSITION,
    MOTOR_VOLUME_CONTROL,
    MOTOR_PRESSURE_CONTROL   
}
Motor_States_e; 

/**
 *  @brief 
 */
void Motor_Init();

/**
 *  @brief 
 */
void Motor_Tasks();

/**
 * @brief 
 */
void Motor_ReturnToHomePosition();

/**
 * @brief 
 * 
 * @param tidalVolume 
 * @param breathPerMinute 
 * @param IE_ratio 
 */
void Motor_VolumeModeSet(uint16_t tidalVolume, uint8_t breathPerMinute, uint8_t IE_ratio);

/**
 * @brief 
 * 
 * @param adjustedPressure 
 * @param breathPerMinute 
 * @param IE_ratio 
 */
void Motor_PressureModeSet(uint16_t adjustedPressure, uint8_t breathPerMinute, uint8_t IE_ratio);

/**
 * @brief 
 * 
 * @return true 
 * @return false 
 */
bool Motor_IsInHomePosition();

/**
 * @brief 
 * 
 * @return uint8_t 
 */
uint8_t Motor_GetBreathPerMinute();

/**
 * @brief 
 * 
 * @return uint8_t 
 */
uint8_t Motor_GetVolumePerMinute();


#endif