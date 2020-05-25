/**
 * @file motor.h
 * @brief Contiene las funciones para el manejo del motor
 * @version 0.1
 */

#ifndef MOTOR_H
#define MOTOR_H

#include <Encoder.h>
#include <Arduino.h>
#include "user_interface.h"

#define MOTOR_STATES_LOG false
#define MOTOR_PID_LOG false

#define MOTOR_GAP_CORRECTION false          // Includes two states to compensate the time it takes for motors to change direction
#define MOTOR_PAUSE_DECELERATION true       // Decreases speed progressively to a complete stop during pause time
#define VOLUME_CONTROL_TRANSITIONS false    // Adds intermediate states in volume control algorithm to change speeds progressively
#define PRESSURE_CONTROL_TRANSITIONS false  // Adds intermediate states in pressure control algorithm to change speeds progressively

/*Control de volumen por flujo en lugar de por cuentas*/
#define VOLUME_FLOW_CONTROL false

/*Velocidades en cuentas/s*/
#define COUNTS_SECOND_SPEEDS true

/*Control activo PID*/
#define CONTROL_ACTIVO_VOLUMEN true
#define CONTROL_ACTIVO_PRESION true
#define CONTROL_SAMPLE_RATE 400  // Hz

/*Limite respiraciones minuto para buffer de datos medidos*/
#define BUFFER_SIZE 30

/*Velocidades*/
#if COUNTS_SECOND_SPEED
    #define VEL_ANG_MAX 2.07    // Experimental en rad/s - 8.986 en el motor del rover ---- 1.52 rad/s a 12V, 1.77 rad/s a 13V, 2.07 rad/s a 14V, 2.54 rad/s a 15V
    #define VEL_ANG_MIN 0.5     // Experimental en rad/s - 3.5 en el motor del rover
    #define VEL_PAUSE 0.2       // Probar
#else
    #define VEL_ANG_MAX 3400    // Experimental en cuentas/s ---- 4087 cuentas/s a 12V, 4760 cuentas/s a 13V, 3400 cuentas/s a 14V, 6830 cuentas/s a 15V
    #define VEL_ANG_MIN 1000    // Experimental en cuentas/s 
    #define VEL_PAUSE 500       // Experimental en cuentas/s
#endif

/*Pausa*/
#define PAUSE_CONTROL_PERIOD    50  // Periodo de control de desaceleracion en pausa en millis
#define MAX_PAUSE_DECELERATION_TIME 100 // Tiempo en milisegundos en que el motor baja de su velocidad a la de pausa

/*Presiones*/
#define PRES_MIN 5          // Minimum control pressure
#define PRES_MAX 35         // Maximum control pressure

/*Volumen*/
#define AIR_LEAK_THRESHOLD 50    // Tolerated measured volume difference between inspiration and expiration in ml
#define VOLUME_COMPENSATION_CONST 0.8   // Constant to multiply error between iterations of VCV to 

/*Encoder*/
#define encoderCountsPerRev 16896       // 8400 en el motor del rover
#define maxVolumeEncoderCounts 3200     // Experimental - 2880 en el motor del rover sacado por proporcion respecto al motor anterior - ToDo medirlo bien
#define minInspirationCounts 100        // Encoder counts needed to determine motor has moved
#define countsFactor 1.0                // Factor to compensate motor does not stop immediately
#define returnDecelerationCounts 300    // Counts threshold when deceleration in RETURN HOME starts

#if MOTOR_GAP_CORRECTION
    #define preparationCounts 100
#else
    #define preparationCounts 0
#endif

/*Motor*/
#define maxMotorCurrent 5.0     //Maximum motor current in amps
#define TIMEOUT_MOTOR_WATCHDOG 500 

/*Definicion de pines*/
#define encoderA 2
#define encoderB 3
#define motorDIR 4
#define motorPWM 5
#define endSwitch 6 
#define pressureSensor A0
#define currentSensor A1

/*Caracteristicas mecanicas*/
#define pistonArea 12271.8463       //En mm2
#define crownRadius 80.00           //En mm

/*Conversiones*/
#define SEC_TO_MILLIS 1000
#define SEC_TO_MICROS 1000000
#define ML_TO_MM3 1000
#define ML_TO_L 1000

typedef enum
{
    MOTOR_STARTING,                 // Motor starting by returning to home
    MOTOR_ADVANCING,                // Motor driving piston forward, airflow to patient
    MOTOR_STOPPED,                  // Motor stopped, pressure mantained during inspiration
    MOTOR_RETURNING,                // Motor returning to home position during expiration
    MOTOR_WAITING                   // Motor stopped in home position expecting cycle trigger
}
Motor_Stages_e;

/**
 * @brief Estructura con todas las variables del motor
 */
typedef struct
{
    bool limitSwitch;       // Switch status
    //Breathing parameters
    float breathsMinute;    // Measured value of breath/minute
    float volumeMinute;     // Volume/minute in the last minute
    float tidalVolume;      // Tidal volume in ml
    float setpointVolume;   // Volume setpoint 
    float inspPercentage;   // Percentage of cycle for inspiration
    float pausePercentage;  // Percentage of inspiration for pause
    uint8_t modeSet;        // Breathing mode set by UI
    bool pressureTrigger;   // Pressure under PEEP to trigger inspiration cycle
    //Angular velocity
    double wSetpoint;       // Angular velocity setpoint for motor 
    double wMeasure;        // Angular velocity measured by encoder in a given period
    double wCommand;        // Angular velocity calculated by PID to minimise error in velocity - Used to drive the motor
    double wDecrement;      // Angular velocity decrement steps to stop motor at the end of pause
    //Pressure
    double pSetpoint;       // Pressure setpoint in Pressure Control Mode
    double pMeasure;        // Pressure measured by sensor in patient airway relative to ambient
    double pCommand;        // Pressure calculated by PID to minimise error in pressure - Used to drive the motor
    bool adjustedPressureReached;   // Adjusted pressure in UI reached by pressure control mode
    //Encoder
    long encoderCounts;     // Counts by encoder in a set period, reset every iteration
    long encoderTotal;      // Total counts by encoder in whole movement
    long inspirationCounts; // Encoder counts to displace tidal volume
    long pauseCounts;       // Encoder counts during pause deceleration
    long prevCounts;        // Encoder counts in last iteration to check motor stopped
    //Times
    float inspirationTime;  // Duration of inspiration in seconds
    float expirationTime;   // Duration of expiration in seconds
    float advanceTime;      // Duration of motor movement in seconds (inspiration-pause)
    float pauseTime;        // Duration of pause in inspiration in seconds
    uint32_t Ts;            // Control loop time period
    uint32_t tAct;          // Auxiliary time variable
    uint32_t tPrev;         // Auxiliary time variable
    uint32_t inspEndTime;   // Time in millis inspiration ends
    uint32_t pauseEndTime;  // Time in millis pause ends
    uint32_t expEndTime;    // Time in millis expiration ends
    uint32_t cycleStart;    // Time in millis a cycle starts
    //Motor state
    Motor_Stages_e motorAction; // Action being carried out by motor
    //Flags
    bool flagInspEnded;     // Flag activated when inspiration ends
    bool flagExpEnded;      // Flag activated when expiration ends
    bool pressureModeFirstIteration;// Flag to make first iteration volume control to calculate PEEP, then pressure mode
    bool movingForwards;            // Flag activated when motor should be moving
    bool movingBackwards;           // Flag activated when motor should be moving
    bool fatalError;                // Fatal error detected by motor watchdog, sends motor to error state
    bool calculateDynamicSpeed;     // Flag to calculate dynamic speed in volume control mode
    //Sensors
    float currentConsumption;   // Current consumed by motor sensed by hall effect sensor
    float expirationVolume;     // Expiration air flow rate
}
MOTOR_t;

extern MOTOR_t MOTOR;

/**
 * @brief FSM states for the motor
 */
typedef enum 
{
    MOTOR_POWER_ON = 0,                 // First state when system starts, does not return to it
    MOTOR_IDLE,                         // Waiting for next cycle to begin
    MOTOR_RETURN_HOME_POSITION,         // Return to home after inspiration or power on
    MOTOR_PREPARE_INSPIRATION,          // Prepare to start inspiration 
    MOTOR_VOLUME_CONTROL,               // Inspiration by volume control
    MOTOR_PRESSURE_CONTROL,             // Inspiration by pressure control
    MOTOR_PREPARE_EXPIRATION,           // Prepare to start expiration
    MOTOR_PAUSE,                        // Pause during inspiration after control condition met (volume displaced) before expiration
    MOTOR_ERROR                         // Error state to send the motor in catastrophic cases

}
Motor_States_e; 


typedef enum
{
    CONTROLLER_FIRST_ACCELERATION = 0,  // Accleration to rise pressure/volume at max speed
    CONTROLLER_FIRST_TRANSITION,        // Transition state between first and second acceleration to make the speed change more gradual
    CONTROLLER_SECOND_ACCELERATION,     // Deceleration to continue to rise pressure/volume at lower speed
    CONTROLLER_SECOND_TRANSITION,       // Transition state between second and maintain acceleration to make the speed change more gradual
    CONTROLLER_MAINTAIN_SETPOINT        // Pressure/volume is maintained at a percentage of VEL_PAUSE with corrections when it lowers
}   
Controller_states_e;


typedef enum
{
    WATCHDOG_IDLE = 0,                  // Motor watchdog in idle state, motor is not moving
    WATCHDOG_MOVE_FORWARDS,             // Motor watchdog checking if motor is moving while set to forwards
    WATCHDOG_MOVE_BACKWARDS,            // Motor watchdog checking if motor is moving while set to backwards
    WATCHDOG_ERROR                      // Motor watchdog detected motor not moving
}
Watchdog_states_e;

typedef struct 
{
    float tidalVolume;              // Volume displaced in a given cycle
    uint32_t cycleTime;             // Time taken by each cycle
    float dynamicCompliance;        // Dynamic pulmonar compliance for each cycle
}
Measured_t;

/**
 * @brief Initializes motor structure variables
 * 
 */
void Motor_Init();

/**
 * @brief Set the physical pins on board
 * 
 */
void setPinModes();

/**
 * @brief Reads the encoder counts since last read
 * 
 */
void lecturaEncoder();

/**
 * @brief Commands motor to move at designed speed
 * 
 * @param dirPin Motor direction pin
 * @param pwmPin Motor PWM pin
 * @param velocidad Angular velocity
 */
void comandoMotor(int dirPin, int pwmPin, double velocidad);

/**
 * @brief Custom pressure control algorithm
 * 
 */
void pressureControlAlgorithm();

/**
 * @brief Custom volume control algorithm
 * 
 */
void volumeControlAlgorithm();

/**
 * @brief Calculates inspiration and expiration times
 * 
 */
void tiemposInspExp();

/**
 * @brief Calculation of encoder counts needed to displace certain volume
 * 
 */
void cuentasEncoderVolumen();

/**
 * @brief State machine for motor movement and control
 * 
 */
void Motor_Tasks();

/**
 * @brief Returns motor to home position indicated by limit switch
 * 
 */
void Motor_ReturnToHomePosition();

/**
 * @brief Called on first iteration of an inspiration cycle, calculates end times and sets MOTOR.inInspiration flag
 * 
 */
void inspirationFirstIteration();

/**
 * @brief Updates breathing parameters at the beginning of a new cycle
 * 
 */
void Motor_SetBreathingParams();

/**
 * @brief Calculates displaced volume from encoder counts in pressure control mode
 * 
 */
float calculateDisplacedVolume();

/**
 * @brief Calculates dynamic pulmonary compliance for each cycle
 * 
 * @return float 
 */
float calculateDynamicCompliance();

/**
 * @brief Saves the measured tidal volume and cycle time in a cicular array
 * 
 * @param measuredTidalVol Measured tidal volume in a cycle
 * @param measuredCycleTime Measured time a cycle takes
 */
void saveRealData(float tidalVol, uint32_t cycleTime, float measuredCompliance);

/**
 * @brief Returns tidal volume of last cycle
 * 
 * @return float Tidal volume
 */
float getTidalVolume();

/**
 * @brief Returns the Breaths in the last Minute
 * 
 * @return float Number of breaths
 */
float getBreathsMinute();

/**
 * @brief Returns the volume sent to patient in the last minute
 * 
 * @return float volume/min
 */
float getVolumeMinute();

/**
 * @brief Get the Dynamic Compliance 
 * 
 * @return float 
 */
float getDynamicCompliance();

/**
 * @brief Updates CTRL control structure with BPM, Tidal vol and Vol minute
 * 
 */
void updateControlVariables();

/**
 * @brief Checks if motor current has surpassed limit
 * 
 */
void checkMotorOvercurrent();

/**
 * @brief Returns system period in microseconds
 * 
 */
void calculateSystemPeriod();

/**
 * @brief Sets alarm if the volume sent is different from the expired
 * 
 */
void compareInspExpVolume();

/**
 * @brief Calculate speed decrements to stop motor at the end of pause
 * 
 */
void calculateDecelerationCurve();

/**
 * @brief Calculates factor to multiply min vel that can be set in pressure mode
 * 
 * @return float 
 */
float minVelPressureFactor();

/**
 * @brief Puts fake data to avoid alarms in buffer
 * 
 */
void fillDataBuffer();

/**
 * @brief 
 * 
 */
void setpointVelocityCalculation();

/**
 * @brief 
 * 
 * @param n 
 * @return true 
 * @return false 
 */
bool MOTOR_Timer(uint32_t n);

/**
 * @brief 
 * 
 * @param n 
 * @return true 
 * @return false 
 */
bool MOTOR_Timer2(uint32_t n);

/**
 * @brief 
 * 
 */
void checkMotorBlocked();


#endif