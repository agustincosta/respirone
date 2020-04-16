#include "sensor.h"

PRESSURE_t pressure[PRESSURE_SENSOR_QTY];

bool plateauPressureDetected;

Sensor_States_e sensorState; 

void Sensor_Init()
{
  sensorState = SENSOR_IDLE;
  plateauPressureDetected = false;
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
      pressure[PRESSURE_SENSOR_1].pValue = (pressure[PRESSURE_SENSOR_1].pValue+1)%PRESSURE_SENSOR_QUEUE_SIZE;
      break;    

    case SENSOR_PROCESS:
      sensorState = SENSOR_IDLE;

      // Average 
      pressure[PRESSURE_SENSOR_1].avgValue = 0;
      for (uint8_t pressureIndex = 0; pressureIndex < PRESSURE_SENSOR_QUEUE_SIZE; pressureIndex++)
      {
        pressure[PRESSURE_SENSOR_1].avgValue = pressure[PRESSURE_SENSOR_1].value[pressureIndex]/PRESSURE_SENSOR_QUEUE_SIZE;    
      }      

      // Plateau detection
      uint8_t pressureCounter;
      for (pressureCounter = 0; pressureCounter < PRESSURE_SENSOR_QUEUE_SIZE; pressureCounter++)
      {
        if (pressure[PRESSURE_SENSOR_1].value[pressureCounter]-pressure[PRESSURE_SENSOR_1].avgValue>PRESSURE_SENSOR_PLATEAU_THRESHOLD)
          break;        
      }
      if (pressureCounter==PRESSURE_SENSOR_QUEUE_SIZE) 
        plateauPressureDetected = true;

      break;

  }      
}

int16_t Sensor_GetLastValue(uint8_t sensorNumber)
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

int16_t Sensor_GetAverageValue(uint8_t sensorNumber)
{
  if (sensorNumber<PRESSURE_SENSOR_QTY)
  {
    return pressure[sensorNumber].avgValue;
  }
  else
  {
    // ToDo: report error
    return PRESSURE_SENSOR_INVALID_VALUE;
  }
}

bool Sensor_PlateauDetected(uint8_t sensorNumber)
{
  if (plateauPressureDetected)
  {
    plateauPressureDetected = false;
    return true;
  }
  else
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