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

/**
 * @brief Estructura con todas las variables del motor
 */
typedef struct
{
    bool limitSwitch;       // Switch status
    //Breathing parameters
    uint8_t breathsMinute;  // Measured value of breath/minute
    uint16_t volumeMinute;  // Volume/minute in the last minute
    uint16_t tidalVolume;   // Tidal volume in ml
    float IE_Ratio;       // Ratio of expirations to inspirations
    bool volumeModeSet;     // Volume control mode set
    bool pressureModeSet;   // Pressure control mode set
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
    long Ts;                // Control loop time period
    long tAct;              // Auxiliary time variable
    long tPrev;             // Auxiliary time variable
    long inspEndTime;       // Time in millis inspiration ends
    long expEndTime;        // Time in millis expiration ends
    //Flags
    bool inInspiration;     // Flag indicating inspiration started
    bool inExpiration;      // Flag indicating expiration started
    bool powerOn;           // Flag indicating system just powered on and needs to return to home position
}
MOTOR_t;

/**
 * @brief FSM states for the motor
 */
typedef enum 
{
    MOTOR_IDLE,
    MOTOR_RETURN_HOME_POSITION,
    MOTOR_VOLUME_CONTROL,
    MOTOR_PRESSURE_CONTROL   
}
Motor_States_e; 

/*Control activo PID*/
#define CONTROL_ACTIVO_VOLUMEN true
#define CONTROL_ACTIVO_PRESION true
#define CONTROL_SAMPLE_RATE 5  // Hz

/*Velocidades*/
#define VEL_ANG_MAX 8.986   // Experimental en rad/s
#define VEL_ANG_MIN 0.65    // Experimental en rad/s - No se usa

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
 * @brief Reads pressure sensor value
 * 
 * @param pressureValue Pressure read 
 * @param offsetADC Offset ADC counts at ambient pressure
 */
void lecturaPresion(double* pressureValue, int offsetADC);

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
 * @brief 
 * 
 * @return true 
 * @return false 
 */
bool Motor_IsInHomePosition();

/**
 * @brief 
 * 
 * @return uint8_t 
 */
uint8_t Motor_GetBreathsPerMinute();

/**
 * @brief 
 * 
 * @return uint8_t 
 */
uint8_t Motor_GetVolumePerMinute();

/**
 * @brief Sets MOTOR.wSetpoint
 * 
 * @param vel Angular velocity setpoint
 */
void setpointVelocity(float vel);

/**
 * @brief Sets parameters for volume control mode
 * 
 * @param tidalVolume 
 * @param breathsPerMinute 
 * @param IE_ratio 
 */
void Motor_VolumeModeSet(uint16_t tidalVolume, uint8_t breathsPerMinute, uint8_t IE_ratio);

/**
 * @brief Sets parameters for pressure control mode
 * 
 * @param adjustedPressure 
 * @param breathsPerMinute 
 * @param IE_ratio 
 */
void Motor_PressureModeSet(uint16_t adjustedPressure, uint8_t breathsPerMinute, uint8_t IE_ratio);

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

#endif