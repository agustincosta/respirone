#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#include "Arduino.h"

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
 * 
 * @param errorNumber 
 */
void DataLogger_ReportError(uint8_t errorNumber);

#endif