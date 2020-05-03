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

// Air pressure
PRESSURE_t pressure;
float pressureAnalogRead;
float pressureRead;
uint8_t pressureReadIndex;

// Air flow
FLOW_t flow;
float flowRead;

// Motor current consumption
CURRENT_t current;
float currentAnalogRead;

// States
Sensor_States_e pressureState;
Sensor_States_e flowState;
Sensor_States_e currentState; 



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
  CTRL.pressure = Pressure_GetLastValue(PRESSURE_SENSOR);
  
  // Inspiration end
  if (MOTOR.flagInspEnded)
  {
    MOTOR.flagInspEnded = false;

    // Update peak pressure value
    CTRL.peakPressure = Pressure_GetPeakValue(PRESSURE_SENSOR);

    // Update plateau pressure value
    if (Pressure_PlateauDetected(PRESSURE_SENSOR))
    {
      CTRL.plateauPressure = Pressure_GetPlateauValue(PRESSURE_SENSOR);
    }
  }

  // Expiration end
  if (MOTOR.flagExpEnded)
  {
    MOTOR.flagExpEnded = false;
    // Update PEEP value
		if (Pressure_PlateauDetected(PRESSURE_SENSOR))
			CTRL.PEEP = Pressure_GetPlateauValue(PRESSURE_SENSOR);
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
  switch(flowState)
  {
    case SENSOR_IDLE:
      if (Sensor_Timer(FLOW_SENSOR_ACQUISITION_PERIOD, FLOW_SENSOR))
      {
        flowState = SENSOR_ACQUIRE;
        Sensor_Timer(SENSOR_START_TIMER, FLOW_SENSOR);

        // Start reading and check 
        if (!Flow_StartReading(FLOW_SENSOR)) UI_SetAlarm(ALARM_NOTE_FLOW_SENSOR_ERROR); 
      }
      break;

    case SENSOR_ACQUIRE:
	    flowState = SENSOR_PROCESS;

	    // Increase pointer
	    flow.pValue = (flow.pValue+1)%FLOW_SENSOR_QUEUE_SIZE;	
      
      // Read
      flowRead = Flow_GetReading(FLOW_SENSOR);

      // Check and queue or set alarm
      if (flowRead==FLOW_SENSOR_INVALID_VALUE) 
      {
        UI_SetAlarm(ALARM_NOTE_FLOW_SENSOR_ERROR);
      }
      else
      {
        current.value[current.pValue] = (flowRead-FLOW_SENSOR_OFFSET_VALUE)/FLOW_SENSOR_FLOW_COEFFICIENT;
      }
      break;    

    case SENSOR_PROCESS:
      flowState = SENSOR_IDLE;

      // Moving average 
      flow.average = 0;
      for (uint8_t flowIndex = 0; flowIndex < FLOW_SENSOR_QUEUE_SIZE; flowIndex++)
      {
        flow.average += flow.value[flowIndex]/ FLOW_SENSOR_QUEUE_SIZE;    
      }      
      break;
  }
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
      current.value[current.pValue] = mapf(currentAnalogRead, SENSOR_ADC_MIN, SENSOR_ADC_MAX, CURRENT_SENSOR_MIN_VALUE, CURRENT_SENSOR_MAX_VALUE)/CURRENT_SENSOR_SENSITIVITY;
      break;    

    case SENSOR_PROCESS:
      currentState = SENSOR_IDLE;

      // Moving average 
      current.average = 0;
      for (uint8_t currentIndex = 0; currentIndex < CURRENT_SENSOR_QUEUE_SIZE; currentIndex++)
      {
        current.average += current.value[currentIndex]/CURRENT_SENSOR_QUEUE_SIZE;    
      }      
      break;
  } 
}

float Pressure_GetLastValue(uint8_t sensorNumber)
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

float Pressure_GetPeakValue(uint8_t sensorNumber)
{
  if (sensorNumber==PRESSURE_SENSOR)
  {
    float returnValue = pressure.peakValue;

    pressure.peakValue = PRESSURE_SENSOR_MIN_VALUE;

    return returnValue;
  }
  else
  {
    // ToDo: report error
    return PRESSURE_SENSOR_INVALID_VALUE;
  }
}

float Pressure_GetPlateauValue(uint8_t sensorNumber)
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

bool Pressure_PlateauDetected(uint8_t sensorNumber)
{
  if (sensorNumber==PRESSURE_SENSOR)
    return pressure.plateauDetected;
  else
  {
  // ToDo: report error
    return PRESSURE_SENSOR_INVALID_VALUE;
  }
}

bool Flow_StartReading(uint8_t sensorNumber)
{
  bool retVal;

  Wire.beginTransmission(FLOW_SENSOR_I2C_ADDRESS); 
  Wire.write(FLOW_SENSOR_I2C_WRITE_CMD);
  Wire.write(FLOW_SENSOR_I2C_GETFLOW_H);
  Wire.write(FLOW_SENSOR_I2C_GETFLOW_L);
  retVal = Wire.endTransmission()?false:true;

  Wire.beginTransmission(FLOW_SENSOR_I2C_ADDRESS); 
  Wire.write(FLOW_SENSOR_I2C_READ_CMD);
  retVal &= Wire.endTransmission()?false:true;

  return retVal;
}

float Flow_GetReading(uint8_t sensorNumber)
{
  uint8_t flowBuffer[2], flowRead_crc8;
  float flowValue;

  Wire.requestFrom(FLOW_SENSOR_I2C_ADDRESS, 3);    
  if (Wire.available()) flowBuffer[1] = Wire.read(); 
  if (Wire.available()) flowBuffer[0] = Wire.read(); 
  if (Wire.available()) flowRead_crc8 = Wire.read(); 

  if (crc8(flowBuffer, 2)==flowRead_crc8)
  {
    flowValue = (float)((flowBuffer[1]<<8) & (flowBuffer[0]));
  }
  else
  {
    flowValue = FLOW_SENSOR_INVALID_VALUE;
  }
  return flowValue;
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

uint8_t crc8(uint8_t data[], uint8_t length)
{
  uint8_t  bit,byte,crc=0x00;

  for(byte=0; byte<length; byte++) 
  {
    crc^=(data[byte]);
    for(bit=8;bit>0;--bit) 
	{
    if(crc&0x80) 
		  crc=(crc<<1)^FLOW_SENSOR_CRC8_POLYNOMIAL; 
	  else
      crc=(crc<<1);
    }
  }
  return crc;
}