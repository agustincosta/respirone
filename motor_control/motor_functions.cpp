#include "motor_functions.h"

//Definicion de encoder
Encoder motor(encoder_a, encoder_b);

//Variables del piston
double diametroPiston = 125; //mm
double radioPiston = diametroPiston/2;
double radioLeva = 34; //mm
int cuentasPorRev = 8400; //Cuentas del encoder por revolución del motor

//Definicion de variables para velocidad motor
double v = 0;               //Velocidad lineal del piston
double w = 0;               //Velocidad angular del motor
double w_medida = 0;        //Velocidad angular del motor medida por encoder
double w_comando = 0;       //Velocidad angular del motor comandada por control
float VEL_ANG_MAX = 8.507;  //Experimental en rad/s
float VEL_ANG_MIN = 0.65;    //Experimental en rad/s
long encoderCount = 0;      //Variable para almacenar la cuenta del encoder


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

void lecturaEncoder(long encoder) {
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
    w = v/radioLeva;

    if(w>VEL_ANG_MAX){
      w = VEL_ANG_MAX;
    }
}

void comandoMotor(int dirPin, int pwmPin) {
  /**
   * Esta funcion setea un nivel pwm segun la velocidad pasada como variables. Se debe especificar el pin de direccion y de pwm del motor DC.
   * 
   * Dependiendo si la velocidad es negativa o positiva se setea el estado del pin de direccion.
   */
   if(v < 0){
      v = -v ;
      digitalWrite(dirPin, LOW); //Controla direccion del motor
   }else{
      digitalWrite(dirPin, HIGH);
   }
   
   if(v <= VELOCIDAD_MINIMA_EN_RAD_S){
    v = 0;
   }

   analogWrite(pwmPin, v/VEL_ANG_MAX*255);
}

void fijarSetPointVelocidad(int frecuenciaRespiracion, double ratio, int volumen) {
  /**
   * Esta funcion setea la velocidad del motor para un ciclo de inspiración en funcion
   * del tiempo disponible y el volumen seteado. Solo para modo control por volumen
   */
  double tiempoInspiratorio = (60/frecuenciaRespiracion)*ratio;
  double factorTiempo = 0.98;
  v = volumen/(PI*(sq(radioPiston))*(tiempoInspiratorio*factorTiempo));
  
}

void calculoDesplazamientoEncoders(long Ts) {
  /**
   * Esta funcion calcula el desplazamiento del piston a partir de la cuenta del encoder
   */
  
  lecturaEncoder(encoderCount);
  
  w_medida = encoderCount*2.0*PI/(cuentasPorRev*Ts);
  
}
