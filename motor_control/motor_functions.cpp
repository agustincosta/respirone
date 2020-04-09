#include "motor_functions.h"

//Definicion de encoder
Encoder motor(encoder_a, encoder_b);

//Definicion de variables para velocidad motor
double w = 0;
double w_medida = 0;
double w_comando = 0;
float VEL_ANG_MAX = 8.507; //Experimental

#if CONTROL_ACTIVO
  double Kp = 0.215, Ki = 4.032, Kd = 0.00; //Variables de PID probadas en Rover
  PID motorPID(&w_medida,&w_comando,&w,Kp, Ki, Kd, DIRECT); //Crea objeto PID
#endif

void initPID() {
  /**
   * Ajusta las variables de PID y la tasa de muestreo. 
   * Se debe llamar en Setup().
   */
  #if CONTROL_ACTIVO
    motorPID.SetMode(AUTOMATIC);

    //Se ajusta el rango de salida porque de lo contrario solo funciona para el rango 0 - 255
    motorPID.SetOutputLimits(-VEL_ANG_MAX*0.98,VEL_ANG_MAX*0.98);

    motorPID.SetSampleTime((1000/(float)CONTROL_SAMPLE_RATE));
  #endif
}

void setPinModes() { 
  /**
   * Setea los pines fisicos, llamar en setup
   */
  pinMode(pwm_motor, OUTPUT);
  pinMode(dir_motor, OUTPUT);
  //El seteo de pines de interrupcion lo hace la libreria encoder
  
}

void lecturaEncoder(long *encoder) {
  /**
   * Utilizando la libreria Encoder se lee el contador de cada rueda y luego se resetea para mantener la cuenta chica.
   */
  encoder = motor.read();
  //resetea el contador del objeto
  motor.write(0);
}

void actualizacionSetPointMotor() {
  /**
   * Se calcula segun el comando recibio de velocidades la velocidad de cada motor.
   */
    //w = ; //

    if(w>VEL_ANG_MAX){
      w = VEL_ANG_MAX;
    }
}

void comandoMotor(int direccion, int pwmPin, float velocidad) {
  /**
   * Esta funcion setea un nivel pwm segun la velocidad pasada como variables. Se debe especificar el pin de direccion y de pwm del motor DC.
   * 
   * Dependiendo si la velocidad es negativa o positiva se setea el estado del pin de direccion.
   */
   
}
