#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include "Arduino.h"

/**
 * @section Defines
 */

#define ARDUINO_PIN_QTY 45

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
#define PROJECT_NAME          "RESPIRONE       "
#define LAUNCH_MENU           "Presione Menu   "
#define SELECT_MODE           "Seleccionar Modo"
#define VOLUME_MODE           "Volumen         "
#define PRESSURE_MODE         "Presion         "
#define AUTO_MODE             "Automatico      "
#define CONFIRMATION          "Confirmar       "
#define AUTO_CONFIRMATION     "Confirmar Auto  "
#define ADJUSTED_PRESSURE     "PA              "
#define TIDAL_VOLUME          "VOL TID         "
#define VOLUME_MINUTE_M       "VMM             "
#define VOLUME_MINUTE_m       "VMm             "
#define RPM                   "RPM             "
#define I_E                   "I:E             "
#define MAX_PRESSURE          "PMax            "
#define TRP                   "TrP             "
#define EMPTY_LINE            "                "

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
  bool setUpComplete;
  uint8_t selectedMode;
  int tidalVolume;
  int volumeMinute_M;
  int volumeMinute_m;
  int rpm;
  int i_e;
  int maxPressure;
  int Trp;
  int adjustedPressure;
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
 * 
 * 
 */
void UI_DisplayMessage(uint8_t pos, uint8_t line, const char * message);


/**
 * 
 */
void UI_DisplayClear();

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
