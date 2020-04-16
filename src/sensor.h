/**
 * @file sensor.h
 * @brief 
 * @version 0.1
 * 
 */

#ifndef SENSOR_H
#define SENSOR_H

#include "Arduino.h"

/**
 * @section Module definitions 
 */
#define PRESSURE_SENSOR_1_PIN               A0

#define PRESSURE_SENSOR_MAX_VALUE           100 // ToDo
#define PRESSURE_SENSOR_MIN_VALUE           0   // ToDo
#define PRESSURE_SENSOR_INVALID_VALUE       PRESSURE_SENSOR_MAX_VALUE+1   
#define PRESSURE_SENSOR_PLATEAU_THRESHOLD   10 // ToDo

#define PRESSURE_SENSOR_ACQUISITION_PERIOD  50

#define PRESSURE_SENSOR_QUEUE_SIZE          10

/**
 * @section Module types 
 */
typedef enum 
{
    PRESSURE_SENSOR_1,
    PRESSURE_SENSOR_QTY
}
PressureSensorIndexes_e; 

typedef struct
{
  int16_t value[PRESSURE_SENSOR_QUEUE_SIZE];        // Measured value
  int16_t avgValue;                                 // Average value
  uint8_t pValue;                                   // Queue index
}
PRESSURE_t;

extern PRESSURE_t pressure[PRESSURE_SENSOR_QTY];

typedef enum 
{
    SENSOR_IDLE,
    SENSOR_ACQUIRE,
    SENSOR_PROCESS
}
Sensor_States_e; 

/**
 *  @brief 
 */
void Sensor_Init();

/**
 * @brief 
 */
void Sensor_Tasks();

/**
 * @brief 
 * 
 * @param sensorNumber 
 * @return int16_t 
 */
int16_t Sensor_GetLastValue(uint8_t sensorNumber);

/**
 * @brief 
 * 
 * @param sensorNumber 
 * @return int16_t 
 */
int16_t Sensor_GetAverageValue(uint8_t sensorNumber);

/**
 * @brief 
 * 
 * @param sensorNumber 
 * @return true 
 * @return false 
 */
bool Sensor_PlateauDetected(uint8_t sensorNumber);

/**
 * @brief 
 * 
 * @param n 
 * @return true 
 * @return false 
 */
bool Sensor_Timer(uint32_t n);

#endif