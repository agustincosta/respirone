#include <Encoder.h>
#include <PID_v1.h>
#include <Arduino.h>

#define CONTROL_ACTIVO true

#define VELOCIDAD_MINIMA_EN_RAD_S 0.65

#define CONTROL_SAMPLE_RATE 500  //hz

/*Definicion de pines*/
#define pwm_motor 9
#define dir_motor 10
#define encoder_a 11
#define encoder_b 12

void initPID();

void setPinModes();

void lecturaEncoder(long encoder);

void actualizacionSetPointMotor();

void comandoMotor(int dirPin, int pwmPin);

void fijarSetPointVelocidad(int frecuenciaRespiracion, double ratio, int volumen);

void calculoDesplazamientoEncoders(long Ts);
