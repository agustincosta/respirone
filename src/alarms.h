#ifndef ALARMS_H
#define ALARMS_H

/**
 * @section Alarm types
 */
enum Alarms
{
  ALARM_OFF,
  ALARM_LOW_BATTERY,           
  ALARM_HIGH_PRESSURE,         
  ALARM_LOW_PRESSURE,          
  ALARM_HIGH_VOLUME_PER_MINUTE,
  ALARM_LOW_VOLUME_PER_MINUTE,
  ALARM_HIGH_BREATHS_PER_MINUTE,
  ALARM_LOW_BREATHS_PER_MINUTE,
  ALARM_PRESSURE_SENSOR_ERROR,
  ALARM_MOTOR_ERROR,
  ALARM_MOTOR_HIGH_CURRENT_CONSUMPTION,
  ALARM_AIR_LEAK
};

/**
 * @section Frequency notes (Hz)	
 */
#define ALARM_NOTE_OFF        						              0
#define ALARM_NOTE_LOW_BATTERY						              100
#define ALARM_NOTE_HIGH_PRESSURE                        100
#define ALARM_NOTE_LOW_PRESSURE                         100
#define ALARM_NOTE_HIGH_VOLUME_PER_MINUTE               100
#define ALARM_NOTE_LOW_VOLUME_PER_MINUTE                100
#define ALARM_NOTE_HIGH_BREATHS_PER_MINUTE              100
#define ALARM_NOTE_LOW_BREATHS_PER_MINUTE               100
#define ALARM_NOTE_PRESSURE_SENSOR_ERROR                100
#define ALARM_NOTE_MOTOR_ERROR                          100
#define ALARM_NOTE_MOTOR_HIGH_CURRENT_CONSUMPTION       100
#define ALARM_NOTE_AIR_LEAK                             100

/**
 * @brief Buffer that stores the frequency note (Hz) of each alarm
 * 
 * @example
 *     buzzerFrequency = alarmNote[ALARM_MOTOR_HIGH_CURRENT_CONSUMPTION];
 */
const uint8_t alarmNote[] =
{
  ALARM_NOTE_OFF,        						   
  ALARM_NOTE_LOW_BATTERY,						   
  ALARM_NOTE_HIGH_PRESSURE,             
  ALARM_NOTE_LOW_PRESSURE,              
  ALARM_NOTE_HIGH_VOLUME_PER_MINUTE,    
  ALARM_NOTE_LOW_VOLUME_PER_MINUTE,     
  ALARM_NOTE_HIGH_BREATHS_PER_MINUTE,   
  ALARM_NOTE_LOW_BREATHS_PER_MINUTE,
  ALARM_NOTE_PRESSURE_SENSOR_ERROR,          
  ALARM_NOTE_MOTOR_ERROR,                    
  ALARM_NOTE_MOTOR_HIGH_CURRENT_CONSUMPTION,
  ALARM_NOTE_AIR_LEAK    
};

/**
 * @section Duration (ms)	
 */
#define ALARM_DURATION_OFF                              100									
#define ALARM_DURATION_LOW_BATTERY                      100
#define ALARM_DURATION_HIGH_PRESSURE                    100
#define ALARM_DURATION_LOW_PRESSURE                     100
#define ALARM_DURATION_HIGH_VOLUME_PER_MINUTE           100
#define ALARM_DURATION_LOW_VOLUME_PER_MINUTE            100
#define ALARM_DURATION_HIGH_BREATHS_PER_MINUTE          100
#define ALARM_DURATION_LOW_BREATHS_PER_MINUTE           100
#define ALARM_DURATION_PRESSURE_SENSOR_ERROR            100
#define ALARM_DURATION_MOTOR_ERROR                      100
#define ALARM_DURATION_MOTOR_HIGH_CURRENT_CONSUMPTION   100
#define ALARM_DURATION_AIR_LEAK                         100

/**
 * @brief Buffer that stores the duration of each alarm
 * 
 * @example
 *     timeout = alarmDuration[ALARM_MOTOR_ERROR];
 */
const uint8_t alarmDuration[] =
{
  ALARM_DURATION_OFF,        						   
  ALARM_DURATION_LOW_BATTERY,						   
  ALARM_DURATION_HIGH_PRESSURE,             
  ALARM_DURATION_LOW_PRESSURE,              
  ALARM_DURATION_HIGH_VOLUME_PER_MINUTE,    
  ALARM_DURATION_LOW_VOLUME_PER_MINUTE,     
  ALARM_DURATION_HIGH_BREATHS_PER_MINUTE,   
  ALARM_DURATION_LOW_BREATHS_PER_MINUTE,
  ALARM_DURATION_PRESSURE_SENSOR_ERROR,         
  ALARM_DURATION_MOTOR_ERROR,                   
  ALARM_DURATION_MOTOR_HIGH_CURRENT_CONSUMPTION,
  ALARM_DURATION_AIR_LEAK   
};

#endif