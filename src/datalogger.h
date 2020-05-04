#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#include "Arduino.h"

#define DATALOG_STATUS_TIMEOUT      100

#define DATALOG_DEFAULT_STATUS      false
#define DATALOG_PRINT_INIT_MESSAGE  false

typedef enum 
{
    ERROR_
}
ErrorNumbers_e; 

/**
 * @brief 
 * 
 */
void DataLogger_Init();

/**
 * @brief 
 * 
 */
void DataLogger_Task();

/**
 * @brief 
 * 
 */
void DataLogger_ReportStatus();

/**
 * @brief 
 */
void DataLogger_PrintUserSettings();

/**
 * @brief 
 * 
 * @param errorNumber 
 */
void DataLogger_ReportError(uint8_t errorNumber);

#endif