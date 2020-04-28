/**
 * @file sensor.cpp
 * @brief 
 * @version 0.1
 */
#include "sensor.h"
#include "control.h"
#include "motor.h"
#include "alarms.h"
#include "analogReadFast.h"

PRESSURE_t pressure[PRESSURE_SENSOR_QTY];

float sensorAnalogRead;
float sensorRead;
uint8_t sensorReadIndex;

Sensor_States_e sensorState; 

void Sensor_Init()
{
  sensorState = SENSOR_IDLE;
  // ToDo> Init struct

  sensorRead = 0;
  sensorReadIndex = 0;  
}

void Sensor_Tasks()
{
  // Update pressure value
  CTRL.pressure = Sensor_GetLastValue(PRESSURE_SENSOR_1);
  
  // Inspiration end
  if (MOTOR.flagInspEnded)
  {
    MOTOR.flagInspEnded = false;

    // Update peak pressure value
    CTRL.peakPressure = Sensor_GetPeakValue(PRESSURE_SENSOR_1);

    // Update plateau pressure value
    if (Sensor_PlateauDetected(PRESSURE_SENSOR_1))
    {
      CTRL.plateauPressure = Sensor_GetPlateauValue(PRESSURE_SENSOR_1);
    }
  }

  // Expiration end
  if (MOTOR.flagExpEnded)
  {
    MOTOR.flagExpEnded = false;
    // Update PEEP value
		if (Sensor_PlateauDetected(PRESSURE_SENSOR_1))
			CTRL.PEEP = Sensor_GetPlateauValue(PRESSURE_SENSOR_1);
  }
  
  // Pressure state machine
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
      // Read, map & add to the moving average queue
      sensorAnalogRead = analogReadFast(PRESSURE_SENSOR_1_PIN);
      sensorRead += map(sensorAnalogRead, SENSOR_ADC_MIN, SENSOR_ADC_MAX, PRESSURE_SENSOR_MIN_VALUE, PRESSURE_SENSOR_MAX_VALUE)/PRESSURE_SENSOR_WINDOW_SIZE;

      if (sensorReadIndex==PRESSURE_SENSOR_WINDOW_SIZE-1)
      {
        sensorState = SENSOR_PROCESS;
         
        // Increase pointer
        pressure[PRESSURE_SENSOR_1].pValue = (pressure[PRESSURE_SENSOR_1].pValue+1)%PRESSURE_SENSOR_QUEUE_SIZE;
        // Queue
        pressure[PRESSURE_SENSOR_1].value[pressure[PRESSURE_SENSOR_1].pValue] = sensorRead;

        sensorReadIndex = 0;
        sensorRead = 0;
      }
      else
      {
        sensorState = SENSOR_IDLE;
        sensorReadIndex++;
      }
      break;    

    case SENSOR_PROCESS:
      sensorState = SENSOR_IDLE;

      // Average 
      pressure[PRESSURE_SENSOR_1].averageValue = 0;
      for (uint8_t pressureIndex = 0; pressureIndex < PRESSURE_SENSOR_QUEUE_SIZE; pressureIndex++)
      {
        pressure[PRESSURE_SENSOR_1].averageValue += pressure[PRESSURE_SENSOR_1].value[pressureIndex];    
      }      
      pressure[PRESSURE_SENSOR_1].averageValue /= PRESSURE_SENSOR_QUEUE_SIZE;

      // Peak
      pressure[PRESSURE_SENSOR_1].peakValue = max(pressure[PRESSURE_SENSOR_1].peakValue, pressure[PRESSURE_SENSOR_1].value[pressure[PRESSURE_SENSOR_1].pValue]);

      // Plateau detection
      uint8_t pressureCounter;
      for (pressureCounter = 0; pressureCounter < PRESSURE_SENSOR_QUEUE_SIZE; pressureCounter++)
      {
        if (abs(pressure[PRESSURE_SENSOR_1].value[pressureCounter]-pressure[PRESSURE_SENSOR_1].averageValue)>PRESSURE_SENSOR_PLATEAU_THRESHOLD)
          break;        
      }
      if (pressureCounter==PRESSURE_SENSOR_QUEUE_SIZE) 
      {
        pressure[PRESSURE_SENSOR_1].plateauDetected = true;
        pressure[PRESSURE_SENSOR_1].plateauValue = pressure[PRESSURE_SENSOR_1].averageValue;
      }

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

int16_t Sensor_GetPeakValue(uint8_t sensorNumber)
{
  if (sensorNumber<PRESSURE_SENSOR_QTY)
  {
    int16_t returnValue = pressure[sensorNumber].peakValue;

    pressure[sensorNumber].peakValue = PRESSURE_SENSOR_MIN_VALUE;

    return returnValue;
  }
  else
  {
    // ToDo: report error
    return PRESSURE_SENSOR_INVALID_VALUE;
  }
}

int16_t Sensor_GetPlateauValue(uint8_t sensorNumber)
{
  if (sensorNumber<PRESSURE_SENSOR_QTY)
  {
    pressure[sensorNumber].plateauDetected = false;
    return pressure[sensorNumber].plateauValue;
  }
  else
  {
    // ToDo: report error
    return PRESSURE_SENSOR_INVALID_VALUE;
  }
}

bool Sensor_PlateauDetected(uint8_t sensorNumber)
{
  return pressure[sensorNumber].plateauDetected;
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