/**
 * @brief Alarm definition header
 */

#ifndef ALARMS_H
#define ALARMS_H

#include "config.h"

/**
 * @brief Alarm types
 * 
 */
enum MedicalAlarms
{
  ALARM_MEDICAL_OFF,        						   

  ALARM_HIGH_PRESSURE,             
  ALARM_LOW_PRESSURE,              
  ALARM_HIGH_VOLUME_PER_MINUTE,    
  ALARM_LOW_VOLUME_PER_MINUTE,     
  ALARM_HIGH_BREATHS_PER_MINUTE,   
  ALARM_LOW_BREATHS_PER_MINUTE,
  ALARM_AIR_LEAK,

  ALARM_MEDICAL_QTY     
};

enum SystemAlarms
{
  ALARM_SYSTEM_OFF,        						   

  ALARM_PRESSURE_SENSOR_ERROR,  
  ALARM_FLOW_SENSOR_ERROR,
  ALARM_CURRENT_SENSOR_ERROR,        
  ALARM_MOTOR_ERROR,                    
  ALARM_MOTOR_HIGH_CURRENT_CONSUMPTION,

  ALARM_SYSTEM_QTY     
};

typedef struct
{
  bool  isActive;                                 // Active alarm flag
  float triggerValue;                             // Trigger value
  float thresholdValue;                           // Threshold value
} 
MedicalAlarm_t;

typedef struct
{
  bool  isActive;                                 //  Active alarm flag
} 
SystemAlarm_t;

typedef struct
{
  bool            enable;                         // Alarm enable (disabled when muted)

  bool            newMedicalEvent;                // New event flag
  MedicalAlarm_t  medical[ALARM_MEDICAL_QTY];     // Medical alarm vector

  bool            newSystemEvent;                 // New event flag
  SystemAlarm_t   system[ALARM_SYSTEM_QTY];       // System alarm vector
} 
ALARM_t;

/**
 * @section Display message for medical alarms	
 */
#define ALARM_MESSAGE_TRIGGER_OFF                               "    "	
#define ALARM_MESSAGE_TRIGGER_HIGH_PRESSURE                     "Pr  "
#define ALARM_MESSAGE_TRIGGER_LOW_PRESSURE                      "Pr  "
#define ALARM_MESSAGE_TRIGGER_HIGH_VOLUME_PER_MINUTE            "VM  "
#define ALARM_MESSAGE_TRIGGER_LOW_VOLUME_PER_MINUTE             "VM  "
#define ALARM_MESSAGE_TRIGGER_HIGH_BREATHS_PER_MINUTE           "RPM "
#define ALARM_MESSAGE_TRIGGER_LOW_BREATHS_PER_MINUTE            "RPM "
#define ALARM_MESSAGE_TRIGGER_ALARM_AIR_LEAK	                  "VT  "

#define ALARM_MESSAGE_THRESHOLD_OFF                             "      "	
#define ALARM_MESSAGE_THRESHOLD_HIGH_PRESSURE                   " PM   "
#define ALARM_MESSAGE_THRESHOLD_LOW_PRESSURE                    " Pm   "
#define ALARM_MESSAGE_THRESHOLD_HIGH_VOLUME_PER_MINUTE          " VMM  "
#define ALARM_MESSAGE_THRESHOLD_LOW_VOLUME_PER_MINUTE           " VMm  "
#define ALARM_MESSAGE_THRESHOLD_HIGH_BREATHS_PER_MINUTE         " RPMM "
#define ALARM_MESSAGE_THRESHOLD_LOW_BREATHS_PER_MINUTE          " RPMm "
#define ALARM_MESSAGE_THRESHOLD_ALARM_AIR_LEAK	                " VTm  "

/**
 * @brief Buffer that stores the display message of each alarm
 * 
 * @example
 *     Print(alarmTriggerMedicalMessage[ALARM_MESSAGE_TRIGGER_LOW_PRESSURE]);
 */
const char alarmTriggerMedicalMessage[ALARM_MEDICAL_QTY][5] =
{
  ALARM_MESSAGE_TRIGGER_OFF,     
  ALARM_MESSAGE_TRIGGER_HIGH_PRESSURE,             
  ALARM_MESSAGE_TRIGGER_LOW_PRESSURE,              
  ALARM_MESSAGE_TRIGGER_HIGH_VOLUME_PER_MINUTE,    
  ALARM_MESSAGE_TRIGGER_LOW_VOLUME_PER_MINUTE,     
  ALARM_MESSAGE_TRIGGER_HIGH_BREATHS_PER_MINUTE,   
  ALARM_MESSAGE_TRIGGER_LOW_BREATHS_PER_MINUTE,
  ALARM_MESSAGE_TRIGGER_ALARM_AIR_LEAK
};

/**
 * @brief Buffer that stores the display message of each alarm
 * 
 * @example
 *     Print(alarmTriggerMedicalMessage[ALARM_MESSAGE_THRESHOLD_LOW_VOLUME_PER_MINUTE]);
 */
const char alarmThresholdMedicalMessage[ALARM_MEDICAL_QTY][7] =
{
  ALARM_MESSAGE_THRESHOLD_OFF,     
  ALARM_MESSAGE_THRESHOLD_HIGH_PRESSURE,             
  ALARM_MESSAGE_THRESHOLD_LOW_PRESSURE,              
  ALARM_MESSAGE_THRESHOLD_HIGH_VOLUME_PER_MINUTE,    
  ALARM_MESSAGE_THRESHOLD_LOW_VOLUME_PER_MINUTE,     
  ALARM_MESSAGE_THRESHOLD_HIGH_BREATHS_PER_MINUTE,   
  ALARM_MESSAGE_THRESHOLD_LOW_BREATHS_PER_MINUTE,
  ALARM_MESSAGE_THRESHOLD_ALARM_AIR_LEAK
};

/**
 * @section Display message for system alarms	
 */
#define ALARM_MESSAGE_OFF                                       "                "
#define ALARM_MESSAGE_PRESSURE_SENSOR_ERROR                     "Codigo: #001    "
#define ALARM_MESSAGE_FLOW_SENSOR_ERROR                         "Codigo: #002    "
#define ALARM_MESSAGE_CURRENT_SENSOR_ERROR                      "Codigo: #003    "
#define ALARM_MESSAGE_MOTOR_ERROR                               "Codigo: #004    "
#define ALARM_MESSAGE_MOTOR_HIGH_CURRENT_CONSUMPTION            "Codigo: #005    "

/**
 * @brief Buffer that stores the display message of each alarm
 * 
 * @example
 *     Print(alarmSystemMessage[ALARM_MESSAGE_MOTOR_ERROR]);
 */
const char alarmSystemMessage[ALARM_SYSTEM_QTY][DISPLAY_COLUMNS+1] =
{
  ALARM_MESSAGE_OFF,     

  ALARM_MESSAGE_PRESSURE_SENSOR_ERROR,        
  ALARM_MESSAGE_FLOW_SENSOR_ERROR,
  ALARM_MESSAGE_CURRENT_SENSOR_ERROR, 
  ALARM_MESSAGE_MOTOR_ERROR,                   
  ALARM_MESSAGE_MOTOR_HIGH_CURRENT_CONSUMPTION
};

#endif