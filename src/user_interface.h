#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include "Arduino.h"

/**
 * @section Defines
 */

#define ARDUINO_PIN_QTY   53

// LEDS
#define LED_ALARM_PIN  11

// Buttons
#define BUTTON_UP_PIN     40   
#define BUTTON_DOWN_PIN   42
#define BUTTON_MENU_PIN   44 
#define BUTTON_ENTER_PIN  46
#define BUTTON_BACK_PIN   48 
 
#define EMERGENCY_STOP    50

// TimeOuts (ms)
#define INIT_MESSAGE_TIMEOUT        3000   
#define TIMEOUT_BLINK               500
#define TIMEOUT_SHOW_SELECTED_PARAM 500  
#define TIMEOUT_RESTART_CONFIG      5000 

//Display messages
//Config
#define DISPLAY_PROJECT_NAME          "RESPIRONE       "
#define DISPLAY_LAUNCH_MENU           "Presione Menu   "
#define DISPLAY_SELECT_MODE           "Seleccionar Modo"
#define DISPLAY_VOLUME_MODE           "Volumen         "
#define DISPLAY_PRESSURE_MODE         "Presion         "
#define DISPLAY_AUTO_MODE             "Automatico      "
#define DISPLAY_MIN_BPM				        "RPMminimo(RPMm) " 	
#define DISPLAY_MAX_BPM				        "RPMMaximo (RPMM)"
#define DISPLAY_ADJUSTED_PRESSURE     "PresAjust (PA)  "
#define DISPLAY_TIDAL_VOLUME          "VolTidal (VT)   "
#define DISPLAY_VOLUME_MINUTE_M       "VolMinuMax (VMM)"
#define DISPLAY_VOLUME_MINUTE_m       "VolMinumin (VMm)"
#define DISPLAY_BPM                   "Resp/Min  (RPM) "
#define DISPLAY_T_I                   "%TiempoIns (TI%)"
#define DISPLAY_T_P                   "%TiempoPau (TP%)"
#define DISPLAY_MAX_PRESSURE          "PresionMax (PM) "
#define DISPLAY_MIN_PRESSURE          "PresionMin (Pm) "
#define DISPLAY_TRP                   "PresionTri (PTr)"	 
#define DISPLAY_CONFIRMATION          "Confirmar       "
#define DISPLAY_AUTO_CONFIRMATION     "Confirmar Auto  "
#define DISPLAY_EMPTY_LINE            "                "
//Show
#define DISPLAY_S_MODE           "SET MD"
#define DISPLAY_R_SND_VOL        "REAL VEn"
#define DISPLAY_S_TIDAL_VOL      "SET VT"
#define DISPLAY_S_ADJ_PRESS      "SET PA"
#define DISPLAY_R_VOL_MIN        "REAL V/m"
#define DISPLAY_R_PRESSURE       "REAL Pr"
#define DISPLAY_R_PEEP           "REAL PP"
#define DISPLAY_S_MAX_PRESS      "SET PM"
#define DISPLAY_R_PEAK_PRESS     "REAL Ppi"
#define DISPLAY_S_BPM            "SET RPM"
#define DISPLAY_R_BPM            "REAL RPM"
#define DISPLAY_S_TI             "SET Ti%"
#define DISPLAY_S_TP             "SET Tp%"
#define DISPLAY_S_TRP            "SET TrP"


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
  UI_SET_RPM_M,
  UI_BLINK_RPM_M,
  UI_DELAY_END_RPM_M,
  UI_SET_RPM_m,
  UI_BLINK_RPM_m,
  UI_DELAY_END_RPM_m,
  UI_SET_T_I,
  UI_BLINK_T_I,
  UI_DELAY_END_T_I,
  UI_SET_T_P,
  UI_BLINK_T_P,
  UI_DELAY_END_T_P,  
  UI_SET_MAX_PRESSURE,
  UI_BLINK_MAX_PRESSURE,
  UI_DELAY_END_MAX_PRESSURE,
  UI_SET_MIN_PRESSURE,
  UI_BLINK_MIN_PRESSURE,
  UI_DELAY_END_MIN_PRESSURE,
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
  UI_SCREEN_1,
  UI_SCREEN_2,
  UI_SCREEN_3,
  UI_SCREEN_4,
  UI_SCREEN_5,
  UI_SCREEN_6,
  UI_SCREEN_7
}
UI_ShowParametersStates_e;

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

  uint8_t t_i,                // Total % of breath cicle to inspiration  
          t_p;                // Total % of breath cicle to pause  

  uint8_t breathsMinute,      // Respiratory rate
          maxBreathsMinute,
          minBreathsMinute;    

  uint16_t adjustedPressure,  // 
           maxPressure,
           minPressure;

  uint16_t tidalVolume,       //
           maxVolumeMinute,
           minVolumeMinute;  

  int8_t TrP;                 // Trigger respect PEEP

  //////////////////////////////////////////////////////////////////////////////////////
  uint8_t i_e;     /////////// SHOULD NOT BE USED ////////////   (just to compile)  ////
  //////////////////////////////////////////////////////////////////////////////////////
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
 *  \return Return description
 *  
 *  \details More details
 */
void UI_DisplayParameters(const char *param1, const char *param2, 
                          const char *val1, uint8_t pos1, const char *val2, uint8_t pos2);

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
 *  \param [in] pin Description for pin
 *  \return Return description
 *  
 *  \details More details
 */
void UI_LoadParam();

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