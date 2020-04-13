#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include "Arduino.h"

/**
 * @section Defines
 */
 // Buttons
#define BUTTON_AUTO_PIN     6   
#define BUTTON_MENU_PIN     5 
#define BUTTON_RIGHT_PIN    4   
#define BUTTON_LEFT_PIN     3 
#define BUTTON_SELECT_PIN   2  

#define EMERGENCY_STOP      7

// TimeOuts (ms)
#define INIT_MESSAGE_TIMEOUT 3000   
#define TIMEOUT_BLINK        500   

//Display messages
#define PROJECT_NAME   "RESPIRONE"
#define LAUNCH_MENU    "Presione Menu"
#define SELECT_MODE    "Seleccionar Modo"
#define VOLUME_MODE    "Volumen"
#define PRESSION_MODE  "Presion"
#define SET_DEFAULT    "Automatico"
#define CONFIRMATION   "Confirmar"
#define EMPTY_LINE     "                "

typedef enum  
{
  UI_WAITING_BUTTON,
  UI_SET_MODE_VOLUME,
  UI_SET_MODE_PRESSION,
  UI_SET_DEFAULT_PARAMETERS,
  UI_BLINK_VOLUME,
  UI_BLINK_PRESSION,
  UI_START,
  UI_WAIT_BUTTON,
  UI_DISPLAY_SUCCESS,
  UI_DISPLAY_ERROR
} 
UI_states_e; 

enum DebounceStates 
{
  WAIT_FOR_BUTTON,
  PRESSED_BUTTON
}; 

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
