/**
 * @file sensor.h
 * @brief 
 * @version 0.1
 */
#ifndef SENSOR_H
#define SENSOR_H

#include "Arduino.h"

/**
 * @section Sensor types
 * 
 */
typedef enum 
{
  PRESSURE_SENSOR,
  FLOW_SENSOR,
  CURRENT_SENSOR,
  SENSORS_QTY
}
SensorIndexes_e; 

/**
 * @section Sensor definitions 
 */

// Acquisition period
#define SENSOR_START_TIMER                  0

// ADCs
#define SENSOR_ADC_MIN                      0
#define SENSOR_ADC_MAX                      1023

// I2C

// Pressure
#define PRESSURE_SENSOR_PIN                 A0
#define PRESSURE_SENSOR_MIN_VALUE           0   
#define PRESSURE_SENSOR_MAX_VALUE           60
#define PRESSURE_SENSOR_INVALID_VALUE       PRESSURE_SENSOR_MAX_VALUE+1.0 
#define PRESSURE_SENSOR_PLATEAU_THRESHOLD   4.0 
#define PRESSURE_SENSOR_ACQUISITION_PERIOD  10
#define PRESSURE_SENSOR_WINDOW_SIZE         4
#define PRESSURE_SENSOR_QUEUE_SIZE          16

// Flow
#define FLOW_SENSOR_ACQUISITION_PERIOD      10 
#define FLOW_SENSOR_QUEUE_SIZE              4

// Current
#define CURRENT_SENSOR_PIN                  A1
#define CURRENT_SENSOR_MIN_VALUE            0   
#define CURRENT_SENSOR_MAX_VALUE            60
#define CURRENT_SENSOR_INVALID_VALUE        PRESSURE_SENSOR_MAX_VALUE+1.0   
#define CURRENT_SENSOR_ACQUISITION_PERIOD   50
#define CURRENT_SENSOR_QUEUE_SIZE           4

/**
 * @section Sensor data types 
 */

// Pressure
typedef struct
{
  float value[PRESSURE_SENSOR_QUEUE_SIZE];        // Measured value queue
  uint8_t pValue;                                 // Queue index
  float average;                                  // Average value
  float peakValue;                                // Peak value
  float plateauValue;                             // Plateau value
  bool plateauDetected;                           // Plateau detected flag
}
PRESSURE_t;

// Flow
typedef struct
{
  float value[FLOW_SENSOR_QUEUE_SIZE];            // Measured value queue
  float average;                                  // Average of the queue values  
  uint8_t pValue;                                 // Queue index
}
FLOW_t;

// Current
typedef struct
{
  float value[CURRENT_SENSOR_QUEUE_SIZE];         // Measured value queue
  float average;                                  // Average of the queue values
  uint8_t pValue;                                 // Queue index
}
CURRENT_t;

// States of acquisition & process FSM
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
 */
void Sensor_PressureInit();

/**
 * @brief 
 */
void Sensor_FlowInit();

/**
 * @brief 
 */
void Sensor_CurrentInit();

/**
 * @brief 
 */
void Sensor_PressureTasks();

/**
 * @brief 
 */
void Sensor_FlowTasks();

/**
 * @brief 
 */
void Sensor_CurrentTasks();

/**
 * @brief 
 * 
 * @param sensorNumber 
 * @return float 
 */
float Sensor_GetLastValue(uint8_t sensorNumber);

/**
 * @brief 
 * 
 * @param sensorNumber 
 * @return float 
 */
float Sensor_GetPeakValue(uint8_t sensorNumber);

/**
 * @brief 
 * 
 * @param sensorNumber 
 * @return float 
 */
float Sensor_GetPlateauValue(uint8_t sensorNumber);

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
 * @param sensor 
 * @return true 
 * @return false 
 */
bool Sensor_Timer(uint32_t n, uint8_t sensor);

#endif