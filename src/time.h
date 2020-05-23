#ifndef TIME_H_INCLUDED
#define TIME_H_INCLUDED

/**
 * Includes
 */
#include "Arduino.h"
#include "stdint.h"

/**
 * Defines
 */
#define MAX_MNT      60
#define MAX_SCN      60
#define MAX_MSCN     1000

#define TIME_TIMEOUT_UPDATE 60000

#define TIME_SCN_ADD 0

// Time enum
typedef enum  
{
    TIME_IDLE,
    TIME_UPDATE
} 
Time_states_e; 

// Time variables
typedef struct
{
  uint32_t hour;     // hour
  uint32_t mnt;      // minute
  uint32_t scn;      // second
  uint32_t mscn;     // millisecond
  uint32_t totalScn; // working seconds 
} 
time_t;

// Global variables
extern time_t working_time,total_time;

/**
 * @brief Init time variables
 */
void Time_Init();

/**
 * @brief Set a new time.
 */
void Time_Save();

/**
 * @brief Get the actual time.
 */
void Time_Get();

/**
 * @brief Get the actual microseconds.
 * @return milliseconds
 */
uint32_t Time_GetMillis();

/**
 * @brief Update the time INC_MS.
 */
void Time_Update_Task();

/**
 * @brief Update time timer.
 */
bool Time_Timer(uint32_t n);

#endif