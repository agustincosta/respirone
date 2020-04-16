#include "sensor.h"

PRESSURE_t pressure[PRESSURE_SENSOR_QTY];

Sensor_States_e sensorState; 

void Sensor_Init()
{
  // pin modes
  sensorState = SENSOR_IDLE;
}

void Sensor_Tasks()
{
  switch(sensorState)
  {
    case SENSOR_IDLE:
      if (Sensor_Timer(PRESSURE_SENSOR_ACQUISITION_PERIOD))
      {
        sensorState = SENSOR_ACQUIRE;
        Sensor_Timer(0);
      }
      break;

    case SENSOR_ACQUIRE:
      sensorState = SENSOR_PROCESS;

      // Acquire and queue
      pressure[PRESSURE_SENSOR_1].value[pressure[PRESSURE_SENSOR_1].pValue] = (uint16_t)map(analogRead(PRESSURE_SENSOR_1_PIN), 0, 1023, PRESSURE_SENSOR_MIN_VALUE, PRESSURE_SENSOR_MAX_VALUE);
      break;    

    case SENSOR_PROCESS:
      sensorState = SENSOR_IDLE;

      // Peak/Plateau detection
      for (uint8_t iValue = pressure[PRESSURE_SENSOR_1].pValue; iValue < PRESSURE_SENSOR_QUEUE_SIZE+pressure[PRESSURE_SENSOR_1].pValue; iValue++)
      {
        //pressure[PRESSURE_SENSOR_1].value[iValue%PRESSURE_SENSOR_QUEUE_SIZE]
      }

      pressure[PRESSURE_SENSOR_1].pValue = (pressure[PRESSURE_SENSOR_1].pValue+1)%PRESSURE_SENSOR_QUEUE_SIZE;
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

bool Sensor_PatientTrigger(uint8_t sensorNumber)
{
  return false;
}

bool Sensor_PlateauDetected(uint8_t sensorNumber)
{
  return false;
}

bool Sensor_Timer(uint32_t n)
{
  static uint32_t initialMillis;

  if(n == 0)
  {
	  initialMillis = millis();
  }
  else if((millis() - initialMillis) > n){
	  return true;
  }
    return false;
}