#include "sensor.h"

PRESSURE_t pressure[PRESSURE_SENSOR_QTY];

void Sensor_Init()
{
  // ToDo
}

void Sensor_ProccessNewValue(uint8_t sensorNumber)
{
  switch (sensorNumber)
  {
    case PRESSURE_SENSOR_1:
      // Acquire and queue
      pressure[PRESSURE_SENSOR_1].value[pressure[PRESSURE_SENSOR_1].pValue] = (uint16_t)map(analogRead(PRESSURE_SENSOR_1_PIN), 0, 1023, PRESSURE_SENSOR_MIN_VALUE, PRESSURE_SENSOR_MAX_VALUE);
      
      // Peak/Plateau detection
      for (uint8_t iValue = pressure[PRESSURE_SENSOR_1].pValue; iValue < PRESSURE_SENSOR_QUEUE_SIZE+pressure[PRESSURE_SENSOR_1].pValue; iValue++)
      {
        //pressure[PRESSURE_SENSOR_1].value[iValue%PRESSURE_SENSOR_QUEUE_SIZE]
      }

      pressure[PRESSURE_SENSOR_1].pValue = (pressure[PRESSURE_SENSOR_1].pValue+1)%PRESSURE_SENSOR_QUEUE_SIZE;
      
      break;

    default:
      break;
  }
}

uint16_t Sensor_GetLastValue(uint8_t sensorNumber)
{
  if (sensorNumber<PRESSURE_SENSOR_QTY)
  {
    return pressure[sensorNumber].value[pressure[sensorNumber].pValue];
  }
  else
  {
    // ToDo: report error
    return PRESSURE_SENSOR_INVALID_VALUE;
  }
}