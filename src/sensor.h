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
#define PRESSURE_SENSOR_INVALID_VALUE       ~0   
#define PRESSURE_SENSOR_PEAK_THRESHOLD      10 // ToDo

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
  uint16_t value[PRESSURE_SENSOR_QUEUE_SIZE];
  uint8_t  pValue;
  uint16_t lastPeakValue;
  uint16_t lastPlateauValue;
}
PRESSURE_t;

extern PRESSURE_t pressure[PRESSURE_SENSOR_QTY];

/**
 *  @brief 
 */
void Sensor_Init();

/**
 * @brief 
 * 
 * @param sensorNumber 
 */
void Sensor_ProccessNewValue(uint8_t sensorNumber);

/**
 * @brief 
 * 
 * @param sensorNumber 
 * @return uint16_t 
 */
uint16_t Sensor_GetLastValue(uint8_t sensorNumber);

#endif