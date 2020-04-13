#include <Encoder.h>
#include <PID_v1.h>
#include <Arduino.h>

#define CONTROL_ACTIVO true

#define VELOCIDAD_MINIMA_EN_RAD_S 0.65

#define CONTROL_SAMPLE_RATE 500  //hz

#define cuentasEncoderRevolucion 8400

/*Definicion de pines*/
#define pwm_motor 5
#define dir_motor 4
#define encoder_a 2
#define encoder_b 3

void initPID();

void setPinModes();

void lecturaEncoder(long* encoderCountAddr);

void comandoMotor(int dirPin, int pwmPin);

void fijarSetPointVelocidad(int frecuenciaRespiracion, double ratio, float volumen, float* recorridoAngularAddr);
