#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include "Arduino.h"

/**
 * @section Defines
 */

#define ARDUINO_PIN_QTY   45

 // Buttons
#define BUTTON_UP_PIN     6   
#define BUTTON_ENTER_PIN  5 
#define BUTTON_BACK_PIN   4   
#define BUTTON_DOWN_PIN   3 
#define BUTTON_MENU_PIN   2  

#define EMERGENCY_STOP      7

// TimeOuts (ms)
#define INIT_MESSAGE_TIMEOUT        3000   
#define TIMEOUT_BLINK               500
#define TIMEOUT_SHOW_SELECTED_PARAM 500  
#define TIMEOUT_RESTART_CONFIG      5000 

//Display messages
#define DISPLAY_PROJECT_NAME          "RESPIRONE       "
#define DISPLAY_LAUNCH_MENU           "Presione Menu   "
#define DISPLAY_SELECT_MODE           "Seleccionar Modo"
#define DISPLAY_VOLUME_MODE           "Volumen         "
#define DISPLAY_PRESSURE_MODE         "Presion         "
#define DISPLAY_AUTO_MODE             "Automatico      "
#define DISPLAY_CONFIRMATION          "Confirmar       "
#define DISPLAY_AUTO_CONFIRMATION     "Confirmar Auto  "
#define DISPLAY_ADJUSTED_PRESSURE     "PA              "
#define DISPLAY_TIDAL_VOLUME          "VOL TID         "
#define DISPLAY_VOLUME_MINUTE_M       "VMM             "
#define DISPLAY_VOLUME_MINUTE_m       "VMm             "
#define DISPLAY_BPM                   "RPM             "
#define DISPLAY_I_E                   "I:E             "
#define DISPLAY_MAX_PRESSURE          "PMax            "
#define DISPLAY_TRP                   "TrP             "
#define DISPLAY_EMPTY_LINE            "                "

typedef enum  
{
  UI_WAITING_BUTTON,
  UI_SET_MODE_AUTO,
  UI_SET_MODE_VOLUME,
  UI_SET_MODE_PRESSURE,
  UI_BLINK_AUTO,
  UI_BLINK_VOLUME,
  UI_BLINK_PRESSURE,
  UI_DELAY_END_MODE,
  UI_SET_TIDAL_VOLUME,
  UI_BLINK_TIDAL_VOLUME,
  UI_DELAY_END_TIDAL_VOLUME,
  UI_SET_ADJUSTED_PRESSURE,
  UI_BLINK_ADJUSTED_PRESSURE,
  UI_DELAY_END_ADJUSTED_PRESSURE,  
  UI_SET_VOLUME_MINUTE_M,
  UI_BLINK_VOLUME_MINUTE_M,
  UI_DELAY_END_VOLUME_MINUTE_M,
  UI_SET_VOLUME_MINUTE_m,
  UI_BLINK_VOLUME_MINUTE_m,
  UI_DELAY_END_VOLUME_MINUTE_m,
  UI_SET_RPM,
  UI_BLINK_RPM,
  UI_DELAY_END_RPM,
  UI_SET_I_E,
  UI_BLINK_I_E,
  UI_DELAY_END_I_E,
  UI_SET_MAX_PRESSURE,
  UI_BLINK_MAX_PRESSURE,
  UI_DELAY_END_MAX_PRESSURE,
  UI_SET_TRP,
  UI_BLINK_TRP,
  UI_DELAY_END_TRP, 
  UI_CONFIRM_CONFIG_PARAMETERS, 
  UI_SET_DEFAULT_PARAMETERS,
  UI_SHOW_PARAMETERS,
  UI_RESTART_CONFIG
} 
UI_states_e; 

typedef enum  
{
  BUTTON_RELEASED,
  BUTTON_PRESSED
}
DebounceStates_t; 


typedef enum  
{
  UI_VOLUME_CONTROL,
  UI_PRESSURE_CONTROL,
  UI_AUTOMATIC_CONTROL
} 
UI_ControlModesOptions_e; 

typedef struct
{
  bool    setUpComplete;      // Setup completed flag
  uint8_t selectedMode;       

  uint8_t i_e;                // Inspiration/Expiration ratio

  uint8_t breathsMinute;      // Respiratory rate
  uint8_t maxBreathsMinute;
  uint8_t minBreathsMinute;    

  uint16_t adjustedPressure;  // 
  uint16_t maxPressure;
  uint16_t minPressure;

  uint16_t tidalVolume;       //
  uint16_t maxVolumeMinute;
  uint16_t minVolumeMinute;  

  int8_t TrP;                 // Trigger respect PEEP
} 
UI_t;

extern UI_t UI;

/**
 *  \brief Brief description
 *  
 *  \return Return description
 *  
 *  \details More details
 */
void UI_Init();

/**
 *  \brief Brief description
 *  
 *  \return Return description
 *  
 *  \details More details
 */
void UI_Task();


/**
 *  \brief Brief description
 *  
 *  \return Return description
 *  
 *  \details More details
 */
void UI_ShowParametersTask();

/**
 *  \brief Brief description
 *  
 *  \param [in] pin Description for pin
 *  \return Return description
 *  
 *  \details More details
 */
bool UI_ButtonDebounce(uint8_t);

/**
 * @brief 
 * 
 * @param pos 
 * @param line 
 * @param message 
 */
void UI_DisplayMessage(uint8_t pos, uint8_t line, const char * message);


/**
 * @brief 
 * 
 */
void UI_DisplayClear();


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