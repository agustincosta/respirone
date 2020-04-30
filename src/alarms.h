/**
 * @brief Alarm definition header
 */

#ifndef ALARMS_H
#define ALARMS_H

#include "config.h"

/**
 * @brief Alarm data types
 */
typedef struct
{
  bool     enable;                                   // Alarm enable (disabled when muted)
  bool     newEvent;                                 // New event flag
  uint8_t  type;                                     // Alarm type
  uint16_t note;                                     // Alarm frequency note (Hz)
  uint16_t duration;                                 // Alarm duration (ms)
  char     message[DISPLAY_COLUMNS+1];               // Alarm message to print or display
} 
ALARM_t;

/**
 * @brief Alarm types
 * 
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
  ALARM_FLOW_SENSOR_ERROR,
  ALARM_CURRENT_SENSOR_ERROR,        
  ALARM_MOTOR_ERROR,                    
  ALARM_MOTOR_HIGH_CURRENT_CONSUMPTION,
  ALARM_AIR_LEAK,
  ALARM_QTY     
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
#define ALARM_NOTE_FLOW_SENSOR_ERROR                    100
#define ALARM_NOTE_CURRENT_SENSOR_ERROR                 100
#define ALARM_NOTE_MOTOR_ERROR                          100
#define ALARM_MOTOR_HIGH_CURRENT_CONSUMPTION            100
#define ALARM_NOTE_ALARM_AIR_LEAK	                      100

/**
 * @brief Buffer that stores the frequency note (Hz) of each alarm
 * 
 * @example
 *     buzzerFrequency = alarmNote[ALARM_MOTOR_HIGH_CURRENT_CONSUMPTION];
 */
const uint16_t alarmNote[ALARM_QTY] =
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
  ALARM_NOTE_FLOW_SENSOR_ERROR,
  ALARM_NOTE_CURRENT_SENSOR_ERROR,        
  ALARM_NOTE_MOTOR_ERROR,                    
  ALARM_MOTOR_HIGH_CURRENT_CONSUMPTION,
  ALARM_NOTE_ALARM_AIR_LEAK     
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
#define ALARM_DURATION_FLOW_SENSOR_ERROR                100
#define ALARM_DURATION_CURRENT_SENSOR_ERROR             100
#define ALARM_DURATION_MOTOR_ERROR                      100
#define ALARM_DURATION_MOTOR_HIGH_CURRENT_CONSUMPTION   100
#define ALARM_DURATION_ALARM_AIR_LEAK	                  100

/**
 * @brief Buffer that stores the duration of each alarm
 * 
 * @example
 *     timeout = alarmDuration[ALARM_MOTOR_ERROR];
 */
const uint16_t alarmDuration[ALARM_QTY] =
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
  ALARM_DURATION_FLOW_SENSOR_ERROR,  
  ALARM_DURATION_CURRENT_SENSOR_ERROR,         
  ALARM_DURATION_MOTOR_ERROR,                   
  ALARM_DURATION_MOTOR_HIGH_CURRENT_CONSUMPTION,
  ALARM_DURATION_ALARM_AIR_LEAK    
};

/**
 * @section Display message 	
 */
#define ALARM_MESSAGE_OFF                              "                "									
#define ALARM_MESSAGE_LOW_BATTERY                      "Bateria baja    "
#define ALARM_MESSAGE_HIGH_PRESSURE                    "Presion alta    "
#define ALARM_MESSAGE_LOW_PRESSURE                     "Presion baja    "
#define ALARM_MESSAGE_HIGH_VOLUME_PER_MINUTE           "Vol/Min alto    "
#define ALARM_MESSAGE_LOW_VOLUME_PER_MINUTE            "Vol/Min bajo    "
#define ALARM_MESSAGE_HIGH_BREATHS_PER_MINUTE          "Rsp/Min alto    "
#define ALARM_MESSAGE_LOW_BREATHS_PER_MINUTE           "Rsp/Min bajo    "
#define ALARM_MESSAGE_PRESSURE_SENSOR_ERROR            "Error: presion  "
#define ALARM_MESSAGE_FLOW_SENSOR_ERROR                "Error: flujo    "
#define ALARM_MESSAGE_CURRENT_SENSOR_ERROR             "Error: corriente"
#define ALARM_MESSAGE_MOTOR_ERROR                      "Error: motor    "
#define ALARM_MESSAGE_MOTOR_HIGH_CURRENT_CONSUMPTION   "Consumo elevado "
#define ALARM_MESSAGE_ALARM_AIR_LEAK	                 "Perdida de aire "

/**
 * @brief Buffer that stores the display message of each alarm
 * 
 * @example
 *     Print(alarmMessage[ALARM_MOTOR_ERROR]);
 */
const char alarmMessage[ALARM_QTY][DISPLAY_COLUMNS+1] =
{
  ALARM_MESSAGE_OFF,        						   
  ALARM_MESSAGE_LOW_BATTERY,						   
  ALARM_MESSAGE_HIGH_PRESSURE,             
  ALARM_MESSAGE_LOW_PRESSURE,              
  ALARM_MESSAGE_HIGH_VOLUME_PER_MINUTE,    
  ALARM_MESSAGE_LOW_VOLUME_PER_MINUTE,     
  ALARM_MESSAGE_HIGH_BREATHS_PER_MINUTE,   
  ALARM_MESSAGE_LOW_BREATHS_PER_MINUTE,
  ALARM_MESSAGE_PRESSURE_SENSOR_ERROR,        
  ALARM_MESSAGE_FLOW_SENSOR_ERROR,
  ALARM_MESSAGE_CURRENT_SENSOR_ERROR, 
  ALARM_MESSAGE_MOTOR_ERROR,                   
  ALARM_MESSAGE_MOTOR_HIGH_CURRENT_CONSUMPTION,
  ALARM_MESSAGE_ALARM_AIR_LEAK
};

#endif