#include <PID_v1.h>

#include <Encoder.h>

#include <Arduino.h>

/*Control activo PID*/
#define CONTROL_ACTIVO_VOLUMEN true
#define CONTROL_ACTIVO_PRESION true
#define CONTROL_SAMPLE_RATE 5  //hz

/*Velocidades*/
#define VEL_ANG_MAX 8.986  //Experimental en rad/s
#define VEL_ANG_MIN 0.65   //Experimental en rad/s - No se usa

/*Encoder*/
#define cuentasEncoderRevolucion 8400

/*Definicion de pines*/
#define pwm_motor 5
#define dir_motor 4
#define encoder_a 2
#define encoder_b 3
#define sensorPresion A0

void initPID();

void setPinModes();

void lecturaEncoder(long* encoderCounter);

void calculoVelocidadMedida(long Ts, long* encoderTotal);

void comandoMotor(int dirPin, int pwmPin, double velocidad);

void setpointVelocidadVolumen(int frecuenciaRespiracion, double ratio, float volumen);

void setpointVelocidadPresion(int presionSeteada);

void lecturaPresion(double* pressureValueAddr, int offsetADC);

void controlDePresion();

void controlDeVolumen(long Ts, long* encoderTotal);

void tiemposInspExp(int frecuenciaRespiracion, double ratio, double* tiempoInspiratorio, double* tiempoExpiratorio);

void inspiracionVolumen(int frecuenciaRespiracion, double ratio, float volumen);
