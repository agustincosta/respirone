/**
 * @file motor.h
 * @brief Contiene las funciones para el manejo del motor
 * @version 0.1
 */

#ifndef MOTOR_H
#define MOTOR_H

#include <PID_v1.h>
#include <Encoder.h>
#include <Arduino.h>
#include "user_interface.h"

#define MOTOR_VERBOSE false

/*Control activo PID*/
#define CONTROL_ACTIVO_VOLUMEN true
#define CONTROL_ACTIVO_PRESION true
#define CONTROL_SAMPLE_RATE 5  // Hz

/*Limite respiraciones minuto*/
#define BUFFER_SIZE 30

/*Velocidades*/
#define VEL_ANG_MAX 8.986   // Experimental en rad/s
#define VEL_ANG_MIN 3.5     // Experimental en rad/s
#define VEL_PAUSE 3.2       // Probar

/*Presiones*/
#define PRES_MIN 5          // Minimum control pressure
#define PRES_MAX 35         // Maximum control pressure

/*Encoder*/
#define encoderCountsPerRev 8400

/*Definicion de pines*/
#define encoderA 2
#define encoderB 3
#define motorDIR 4
#define motorPWM 5
#define endSwitch 6 
#define pressureSensor A0

/*Caracteristicas mecanicas*/
#define pistonArea 12271.8463   //En mm2
#define crownRadius 31.50       //En mm

/*Conversiones*/
#define SEC_TO_MILLIS 1000
#define ML_TO_MM3 1000

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
    uint8_t breathsMinute;  // Measured value of breath/minute
    uint16_t volumeMinute;  // Volume/minute in the last minute
    uint32_t tidalVolume;   // Tidal volume in ml
    float inspPercentage;   // Percentage of cycle for inspiration
    float pausePercentage;  // Percentage of inspiration for pause
    uint8_t modeSet;        // Breathing mode set by UI
    bool pressureTrigger;   // Pressure under PEEP to trigger inspiration cycle
    //Angular velocity
    double wSetpoint;       // Angular velocity setpoint for motor 
    double wMeasure;        // Angular velocity measured by encoder in a given period
    double wCommand;        // Angular velocity calculated by PID to minimise error in velocity - Used to drive the motor
    //Pressure
    double pSetpoint;       // Pressure setpoint in Pressure Control Mode
    double pMeasure;        // Pressure measured by sensor in patient airway relative to ambient
    double pCommand;        // Pressure calculated by PID to minimise error in pressure - Used to drive the motor
    //Encoder
    long encoderCounts;     // Counts by encoder in a set period, reset every iteration
    long encoderTotal;      // Total counts by encoder in whole movement
    long inspirationCounts; // Encoder counts to displace tidal volume
    //Times
    float inspirationTime;  // Duration of inspiration in seconds
    float expirationTime;   // Duration of expiration in seconds
    uint32_t Ts;            // Control loop time period
    uint32_t tAct;          // Auxiliary time variable
    uint32_t tPrev;         // Auxiliary time variable
    uint32_t inspEndTime;   // Time in millis inspiration ends
    uint32_t expEndTime;    // Time in millis expiration ends
    uint32_t cycleStart;    // Time in millis a cycle starts
    //Motor state
    Motor_Stages_e motorAction; // Action being carried out by motor
    //Flags
    bool flagInspEnded;     // Flag activated when inspiration ends
    bool flagExpEnded;      // Flag activated when expiration ends
}
MOTOR_t;

extern MOTOR_t MOTOR;

/**
 * @brief FSM states for the motor
 */
typedef enum 
{
    MOTOR_POWER_ON,                 // First state when system starts, does not return to it
    MOTOR_IDLE,                     // Waiting for next cycle to begin
    MOTOR_RETURN_HOME_POSITION,     // Return to home after inspiration or power on
    MOTOR_VOLUME_CONTROL,           // Inspiration by volume control
    MOTOR_PRESSURE_CONTROL,         // Inspiration by pressure control
    MOTOR_PAUSE,                    // Pause during inspiration after control condition met (volume displaced) before expiration
}
Motor_States_e; 

typedef struct 
{
    float tidalVolume;           // Volume displaced in a given cycle
    uint32_t cycleTime;             // Time taken by each cycle
}
Measured_t;

/**
 * @brief Initializes motor structure variables
 * 
 */
void Motor_Init();

/**
 * @brief Initializes pressure and volume PIDs
 * 
 */
void initPID();

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
 * @brief Calculation of angular velocity measured by encoder
 * 
 * @param Ts Period of time corresponding to measure
 * @param encoderTotal Encoder counts in period of time
 */
void calculoVelocidadMedida(long Ts);

/**
 * @brief Commands motor to move at designed speed
 * 
 * @param dirPin Motor direction pin
 * @param pwmPin Motor PWM pin
 * @param velocidad Angular velocity
 */
void comandoMotor(int dirPin, int pwmPin, double velocidad);

/**
 * @brief Controls the pressure of system using PID
 * 
 */
void controlDePresion();

/**
 * @brief Controls the volume of system using PID
 * 
 * @param Ts Control period
 */
void controlDeVolumen();

/**
 * @brief Calculates inspiration and expiration times
 * 
 */
void tiemposInspExp();

/**
 * @brief Function called in MOTOR_VOLUME_CONTROL mode
 *  
 * @param Ts Control period
 */
void inspiracionVolumen();

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
 * @brief Sets MOTOR.wSetpoint taking into account UI.t_i and UI.t_p
 * 
 */
void setpointVelocityCalculation();

/**
 * @brief Control period calculation
 * 
 */
void calculateControlPeriod();

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
 * @brief Saves the measured tidal volume and cycle time in a cicular array
 * 
 * @param measuredTidalVol Measured tidal volume in a cycle
 * @param measuredCycleTime Measured time a cycle takes
 */
void saveRealData(float tidalVol, uint32_t cycleTime);

/**
 * @brief Returns tidal volume of last cycle
 * 
 * @return float Tidal volume
 */
float getTidalVolume();

/**
 * @brief Returns the Breaths in the last Minute
 * 
 * @return uint8_t Number of breaths
 */
uint8_t getBreathsMinute();

/**
 * @brief Returns the volume sent to patient in the last minute
 * 
 * @return float volume/min
 */
float getVolumeMinute();

/**
 * @brief Updates CTRL control structure with BPM, Tidal vol and Vol minute
 * 
 */
void updateControlVariables();

#endif