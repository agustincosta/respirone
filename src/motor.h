/**
 * @file motor.h
 * @brief 
 * @version 0.1
 */
#ifndef MOTOR_H
#define MOTOR_H

#include "Arduino.h"

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
 * 
 * @return true 
 * @return false 
 */
bool Motor_IsInHomePosition();

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