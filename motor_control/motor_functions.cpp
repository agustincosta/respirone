#include "motor_functions.h"

//Definicion de encoder
Encoder motor(encoder_a, encoder_b);

//Variables del piston
float diametroPiston = 125; //mm
float radioPiston = diametroPiston/2;
float areaPiston = 12271.8463; //mm2
float radioLeva = 34; //mm
int cuentasPorRev = 8400; //Cuentas del encoder por revolución del motor

//Definicion de variables para velocidad motor
float v = 0;               //Velocidad lineal del piston
double w = 0;               //Velocidad angular del motor
double w_medida = 0;        //Velocidad angular del motor medida por encoder
double w_comando = 0;       //Velocidad angular del motor comandada por control
float VEL_ANG_MAX = 8.976;  //Experimental en rad/s
float VEL_ANG_MIN = 0.65;    //Experimental en rad/s
//long encoderCount = 0;      //Variable para almacenar la cuenta del encoder
long encoderPrev = -999;


//Variables de PID
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

void lecturaEncoder(long* encoderCountAddr) {
  /**
   * Utilizando la libreria Encoder se lee el contador de cada rueda y luego se resetea para mantener la cuenta chica.
   */
  *encoderCountAddr = motor.read();
  
  if (*encoderCountAddr != encoderPrev) {
    encoderPrev = *encoderCountAddr;
  }
}

void comandoMotor(int dirPin, int pwmPin) {
  /**
   * Esta funcion setea un nivel pwm segun la velocidad pasada como variables. Se debe especificar el pin de direccion y de pwm del motor DC.
   * 
   * Dependiendo si la velocidad es negativa o positiva se setea el estado del pin de direccion.
   */
   bool direccion = true;
   if(w < 0)
   {
      w = -w ;
      digitalWrite(dirPin, HIGH); //Controla direccion del motor
      direccion = true;
   }
   else
   {
      digitalWrite(dirPin, LOW);
      direccion = false;
      w=w*0.97;
   }
   
   //Serial.print("PWM: "); Serial.println(w/VEL_ANG_MAX*255);
   //Serial.print("DIR: "); Serial.println(direccion);
   analogWrite(pwmPin, w/VEL_ANG_MAX*255);
}

void fijarSetPointVelocidad(int frecuenciaRespiracion, double ratio, float volumen, float* recorridoAngularAddr) {
  /**
   * Esta funcion setea la velocidad del motor para un ciclo de inspiración en funcion
   * del tiempo disponible y el volumen seteado. Solo para modo control por volumen
   */
  double tiempoInspiratorio = (60/frecuenciaRespiracion)*ratio; //En segundos
  //Serial.print("Tiempo: "); Serial.println(tiempoInspiratorio);
  double factorTiempo = 1;
  v = volumen/(areaPiston*tiempoInspiratorio*factorTiempo); //todo en unidades del SI    
  
  //Serial.print("velocidad: "); Serial.println(v);  //En mm/s

  *recorridoAngularAddr = (volumen/areaPiston)/0.4539;

  w = v*3.0677;         //En deg/s, el 3.0677 es medio turbio, sale de 125° en 1,33s
  w = 2*PI*w/360;       //En rad/s
  //Serial.print("omega (rad/s): "); Serial.println(w);
}
