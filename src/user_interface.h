#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include "Arduino.h"

/**
 * @section Defines
 */
 // Buttons
#define BUTTON_OK_PIN     6   
#define BUTTON_STOP_PIN   6 
#define BUTTON_UP_PIN     6   
#define BUTTON_RIGHT_PIN  6  
#define BUTTON_LEFT_PIN   6  
#define BUTTON_DOWN_PIN   6  

// TimeOuts
#define INIT_MESSAGE_TIMEOUT 3000   //ms

typedef enum  
{
  UI_INIT_MESSAGE,
  UI_START,
  UI_WAIT_BUTTON,
  UI_DISPLAY_SUCCESS,
  UI_DISPLAY_ERROR
} 
UI_states_e; 

typedef enum  
{
  UI_VOLUME_CONTROL,
  UI_PRESSURE_CONTROL
} 
UI_ControlModesOptions_e; 

typedef struct
{
  bool    setUpComplete;
  uint8_t selectedMode;

  uint8_t IE_ratio;

  uint8_t breathPerMinute;
  uint8_t maxBreathPerMinute;
  uint8_t minBreathPerMinute;    

  uint16_t adjustedPressure;
  uint16_t maxPressure;
  uint16_t minPressure;

  uint16_t tidalVolume;
  uint16_t maxVolumePerMinute;
  uint16_t minVolumePerMinute;  

  uint8_t triggerRespectPEEP;  
} 
UI_t;

extern UI_t UI;

typedef enum  
{
  BUTTON_PRESSED,
  BUTTON_RELEASED
}
DebounceStates_t; 

/**
 *  \brief Brief description
 *  
 *  \return Return description
 *  
 *  \details More details
 */
void UserInterface_Init();

/**
 *  \brief Brief description
 *  
 *  \return Return description
 *  
 *  \details More details
 */
void UserInterface_Task();

/**
 *  \brief Brief description
 *  
 *  \param [in] pin Description for pin
 *  \return Return description
 *  
 *  \details More details
 */
bool UI_ButtonDebounce(uint8_t pin);

/**
 * @brief 
 * 
 * @param alarm 
 */
void UI_SetAlarm(uint8_t alarm);

/**
 *  \brief Brief description
 *  
 *  \param [in] n Description for n
 *  \return Return description
 *  
 *  \details More details
 */
bool UI_Timer(uint32_t n);

#endif // USER_INTERFACE_H
