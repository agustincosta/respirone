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
#include "Wire.h"

PRESSURE_t pressure;
float pressureAnalogRead;
float pressureRead;
uint8_t pressureReadIndex;

FLOW_t flow;

CURRENT_t current;
float currentAnalogRead;

// States
Sensor_States_e pressureState;
Sensor_States_e flowState;
Sensor_States_e currentState; 

// Map for floats
float mapf(float val, float in_min, float in_max, float out_min, float out_max);

void Sensor_Init()
{
  Sensor_PressureInit();
  Sensor_FlowInit();
  Sensor_CurrentInit();
}

void Sensor_Tasks()
{
  Sensor_PressureTasks();
  Sensor_FlowTasks();
  Sensor_CurrentTasks();
}

void Sensor_PressureInit()
{
  pressureState = SENSOR_IDLE;
  // ToDo> Init struct

  pressureRead = 0;
  pressureReadIndex = 0;  
}

void Sensor_FlowInit()
{
  flowState = SENSOR_IDLE;
  // ToDo> Init struct
  Wire.begin();
}

void Sensor_CurrentInit()
{
  currentState = SENSOR_IDLE;
  // ToDo> Init struct
}

void Sensor_PressureTasks()
{
  // Update pressure value
  CTRL.pressure = Sensor_GetLastValue(PRESSURE_SENSOR);
  
  // Inspiration end
  if (MOTOR.flagInspEnded)
  {
    MOTOR.flagInspEnded = false;

    // Update peak pressure value
    CTRL.peakPressure = Sensor_GetPeakValue(PRESSURE_SENSOR);

    // Update plateau pressure value
    if (Sensor_PlateauDetected(PRESSURE_SENSOR))
    {
      CTRL.plateauPressure = Sensor_GetPlateauValue(PRESSURE_SENSOR);
    }
  }

  // Expiration end
  if (MOTOR.flagExpEnded)
  {
    MOTOR.flagExpEnded = false;
    // Update PEEP value
		if (Sensor_PlateauDetected(PRESSURE_SENSOR))
			CTRL.PEEP = Sensor_GetPlateauValue(PRESSURE_SENSOR);
  }
  
  // Pressure state machine
  switch(pressureState)
  {
    case SENSOR_IDLE:
      if (Sensor_Timer(PRESSURE_SENSOR_ACQUISITION_PERIOD, PRESSURE_SENSOR))
      {
        pressureState = SENSOR_ACQUIRE;
        Sensor_Timer(SENSOR_START_TIMER, PRESSURE_SENSOR);
      }
      break;

    case SENSOR_ACQUIRE:
      // Read, map & add to the moving average queue
      pressureAnalogRead = (float)analogReadFast(PRESSURE_SENSOR_PIN);
      pressureRead += mapf(pressureAnalogRead, SENSOR_ADC_MIN, SENSOR_ADC_MAX, PRESSURE_SENSOR_MIN_VALUE, PRESSURE_SENSOR_MAX_VALUE)/PRESSURE_SENSOR_WINDOW_SIZE;

      if (pressureReadIndex==PRESSURE_SENSOR_WINDOW_SIZE-1)
      {
        pressureState = SENSOR_PROCESS;
         
        // Increase pointer
        pressure.pValue = (pressure.pValue+1)%PRESSURE_SENSOR_QUEUE_SIZE;
        // Queue
        pressure.value[pressure.pValue] = pressureRead;  

        pressureReadIndex = 0;
        pressureRead = 0;
      }
      else
      {
        pressureState = SENSOR_IDLE;
        pressureReadIndex++;
      }
      break;    

    case SENSOR_PROCESS:
      pressureState = SENSOR_IDLE;

      // Average 
      pressure.average = 0;
      for (uint8_t pressureIndex = 0; pressureIndex < PRESSURE_SENSOR_QUEUE_SIZE; pressureIndex++)
      {
        pressure.average += pressure.value[pressureIndex];    
      }      
      pressure.average /= PRESSURE_SENSOR_QUEUE_SIZE;

      // Peak
      pressure.peakValue = max(pressure.peakValue, pressure.value[pressure.pValue]);

      // Plateau detection
      uint8_t pressureCounter;
      for (pressureCounter = 0; pressureCounter < PRESSURE_SENSOR_QUEUE_SIZE; pressureCounter++)
      {
        if (abs(pressure.value[pressureCounter]-pressure.average)>PRESSURE_SENSOR_PLATEAU_THRESHOLD)
          break;        
      }
      if (pressureCounter==PRESSURE_SENSOR_QUEUE_SIZE) 
      {
        pressure.plateauDetected = true;
        pressure.plateauValue = pressure.average;
      }

      break;
  }      
}

void Sensor_FlowTasks()
{

}

void Sensor_CurrentTasks()
{
  switch(currentState)
  {
    case SENSOR_IDLE:
      if (Sensor_Timer(CURRENT_SENSOR_ACQUISITION_PERIOD, CURRENT_SENSOR))
      {
        currentState = SENSOR_ACQUIRE;
        Sensor_Timer(SENSOR_START_TIMER, CURRENT_SENSOR);
      }
      break;

    case SENSOR_ACQUIRE:
	    currentState = SENSOR_PROCESS;
	  
	    // Increase pointer
	    current.pValue = (current.pValue+1)%CURRENT_SENSOR_QUEUE_SIZE;	
      // Read, map & queue
      currentAnalogRead = (float)analogReadFast(CURRENT_SENSOR_PIN);
      current.value[current.pValue] = mapf(currentAnalogRead, SENSOR_ADC_MIN, SENSOR_ADC_MAX, CURRENT_SENSOR_MIN_VALUE, CURRENT_SENSOR_MAX_VALUE)/CURRENT_SENSOR_QUEUE_SIZE;
      break;    

    case SENSOR_PROCESS:
      currentState = SENSOR_IDLE;

      // Moving average 
      current.average = 0;
      for (uint8_t currentIndex = 0; currentIndex < CURRENT_SENSOR_QUEUE_SIZE; currentIndex++)
      {
        current.average += current.value[currentIndex]/= CURRENT_SENSOR_QUEUE_SIZE;    
      }      
      break;
  } 
}

float Sensor_GetLastValue(uint8_t sensorNumber)
{
  if (sensorNumber==PRESSURE_SENSOR)
  {
    return pressure.average;
  }
  else
  {
    // ToDo: report error
    return PRESSURE_SENSOR_INVALID_VALUE;
  }
}

float Sensor_GetPeakValue(uint8_t sensorNumber)
{
  if (sensorNumber==PRESSURE_SENSOR)
  {
    int16_t returnValue = pressure.peakValue;

    pressure.peakValue = PRESSURE_SENSOR_MIN_VALUE;

    return returnValue;
  }
  else
  {
    // ToDo: report error
    return PRESSURE_SENSOR_INVALID_VALUE;
  }
}

float Sensor_GetPlateauValue(uint8_t sensorNumber)
{
  if (sensorNumber==PRESSURE_SENSOR)
  {
    pressure.plateauDetected = false;
    return pressure.plateauValue;
  }
  else
  {
    // ToDo: report error
    return PRESSURE_SENSOR_INVALID_VALUE;
  }
}

bool Sensor_PlateauDetected(uint8_t sensorNumber)
{
  if (sensorNumber==PRESSURE_SENSOR)
    return pressure.plateauDetected;
  else
  {
  // ToDo: report error
    return PRESSURE_SENSOR_INVALID_VALUE;
  }
    
}

bool Sensor_Timer(uint32_t n, uint8_t sensor)
{
  static uint32_t initialMillis[SENSORS_QTY];

  if(n == 0)
  {
	  initialMillis[sensor] = millis();
  }
  else if((millis() - initialMillis[sensor]) > n){
	  return true;
  }
  return false;
}

float mapf(float val, float in_min, float in_max, float out_min, float out_max)
{
    return (val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}