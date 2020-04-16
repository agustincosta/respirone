#include "motor_functions.h"

//Definicion de encoder
Encoder motor(encoder_a, encoder_b);

//Variables del piston
float diametroPiston = 125; //mm
float radioPiston = diametroPiston/2;
float areaPiston = 12271.8463; //mm2
float radioLeva = 0.0315; //mm
int cuentasPorRev = 8400; //Cuentas del encoder por revolución del motor

//Definicion de variables para velocidad motor
float v = 0;                //Velocidad lineal del piston
double w_setpoint= 0;       //Velocidad angular del motor
double w_medida = 0;        //Velocidad angular del motor medida por encoder
double w_comando = 0;       //Velocidad angular del motor comandada por control

//Definicion de variables para encoder
long encoderPrev = -999;
long encoderCounts = 0;      //Variable que almacena la cuenta del encoder

//Definicion de variables para presion del piston, todo en cmH2O
double p_medida = 0;             //Presion medida por el sensor
double p_comando = 0;            //Presion de salida del PID
double p_setpoint = 0;           //Presion de setpoint
long presionMinimaControl = 5;  //Presion minima para control
long presionMaximaControl = 35; //Presion maxima para control


/*Variables de PID*/ 
//Ambos usan las mismas
double Kp = 0.215, Ki = 4.032, Kd = 0.00; //Variables de PID probadas en Rover


//PID de control por volumen
PID volumenPID(&w_medida,&w_comando,&w_setpoint, Kp, Ki, Kd, DIRECT); //Crea objeto PID

//PID de control por presion
PID presionPID(&p_medida,&p_comando,&p_setpoint, Kp, Ki, Kd, DIRECT); //Crea objeto PID


void initPID() {
  /**
   * Ajusta las variables de PID y la tasa de muestreo. 
   * Se debe llamar en Setup().
   */

  //PID para control por volumen
  #if CONTROL_ACTIVO_VOLUMEN
  {
    volumenPID.SetMode(AUTOMATIC);

    //Se ajusta el rango de salida porque de lo contrario solo funciona para el rango 0 - 255
    volumenPID.SetOutputLimits(-VEL_ANG_MAX*0.98,VEL_ANG_MAX*0.98);

    volumenPID.SetSampleTime((1000/(float)CONTROL_SAMPLE_RATE));
  }
  #endif

  //PID para control por presion
  #if CONTROL_ACTIVO_PRESION
  {
    presionPID.SetMode(AUTOMATIC);

    //Se ajusta el rango de salida porque de lo contrario solo funciona para el rango 0 - 255
    presionPID.SetOutputLimits(presionMinimaControl, presionMaximaControl);

    presionPID.SetSampleTime((1000/(float)CONTROL_SAMPLE_RATE));
  }
  #endif

}

void setPinModes() { 
  /**
   * Setea los pines fisicos, llamar en setup
   */
  pinMode(pwm_motor, OUTPUT);
  pinMode(dir_motor, OUTPUT);
  //El seteo de pines de interrupcion lo hace la libreria encoder
  //No hay que setear nada para el analogico
  
}

void lecturaEncoder(long* encoderCounter) {
  /**
   * Utilizando la libreria Encoder se lee el contador de cada rueda y luego se resetea
   * para mantener las cuentas de solo un periodo.
   */
  *encoderCounter = motor.read();

  motor.write(0);
}

void calculoVelocidadMedida(long Ts, long* encoderTotal) {
  /**
   * Calcula la velocidad angular del periodo dado por el tiempo Ts en ms
   */
  lecturaEncoder(&encoderCounts);
  *encoderTotal += encoderCounts;

  w_medida = encoderCounts*2.0*PI/(cuentasPorRev*Ts/1000); //En rad/s
   
}

void comandoMotor(int dirPin, int pwmPin, double velocidad) {
  /**
   * Esta funcion setea un nivel pwm segun la velocidad pasada como variables. Se debe especificar el pin de direccion y de pwm del motor DC.
   * 
   * Dependiendo si la velocidad es negativa o positiva se setea el estado del pin de direccion.
   */
   if(velocidad < 0)
   {
      velocidad = -velocidad ;
      digitalWrite(dirPin, LOW); //Controla direccion del motor
   }
   else
   {
      digitalWrite(dirPin, HIGH);
   }
   
   //Serial.print("PWM: "); Serial.println(velocidad/VEL_ANG_MAX*255);
   //Serial.print("DIR: "); Serial.println(direccion);
   analogWrite(pwmPin, velocidad/VEL_ANG_MAX*255);
}

void setpointVelocidadVolumen(int frecuenciaRespiracion, double ratio, float volumen) {
  /**
   * Esta funcion setea la velocidad del motor para un ciclo de inspiración en funcion
   * del tiempo disponible y el volumen seteado. Solo para modo control por volumen
   */
  double tiempoInspiratorio = (60.0/frecuenciaRespiracion)*ratio; //En segundos
  //Serial.print("Tiempo: "); Serial.println(tiempoInspiratorio);
  double factorTiempo = 0.90;
  v = volumen/(areaPiston*tiempoInspiratorio*factorTiempo); //todo en unidades del SI    
  
  //Serial.print("velocidad: "); Serial.println(v);  //En mm/s

  float recorridoAngularAddr = (volumen/areaPiston)/0.4539;

  w_setpoint = v*3.0677;         //En deg/s, el 3.0677 es medio turbio, sale de 125° en 1,33s
  w_setpoint = 2.0*PI*w_setpoint/360.0;       //En rad/s
  //Serial.print("omega (rad/s): "); Serial.println(w);
}

void setpointVelocidadPresion(int presionSeteada) {
  /**
   * Esta funcion fija la velocidad con la que comienza el motor para el modo por presion
   * Es una estimacion muy basica para tener una velocidad con la que comienza 
   */
   w_comando = map(presionSeteada, presionMinimaControl, presionMaximaControl, 36, 255);
}

void lecturaPresion(double* pressureValueAddr, int offsetADC) {
  /**
   * Esta funcion lee el valor de presion del sensor a traves del ADC y corrige la 
   * medida restando el offset que tenga el ADC a presion ambiente
   */
  int valorADC = analogRead(sensorPresion);
  *pressureValueAddr = map(valorADC-offsetADC, 0, 1023, 0, 600);  //Conversion a mbar
}

void controlDePresion() {
  /**
   * Esta funcion usa un PID para computar la presion de comando para alcanzar 
   * y determina la velocidad del motor para alcanzarla
   */
   double deltaPresion = 0;
   double deltaVelocidad = 0; 
   
   lecturaPresion(&p_medida, 10);   //Actualiza p_medida
   presionPID.Compute();            //Actualiza p_comando en funcion de p_medida y p_setpoint
   deltaPresion = p_setpoint - p_medida;   //Diferencia de presion para determinar cambio de velocidad
   deltaVelocidad = map(abs(deltaPresion), 0, 30, 0, 255);   //Convierte diferencia de presion a diferencia de velocidad (TURBIO)
   
   if (deltaPresion >= 0) 
   {
     w_comando = w_comando + deltaVelocidad;  //Modifica velocidad
   }
   else 
   {
     w_comando = w_comando - deltaVelocidad;
   }

   comandoMotor(dir_motor, pwm_motor, w_comando); //Mueve el motor
}

void controlDeVolumen(long Ts, long* encoderTotal) {
  /**
   * Esta funcion usa un PID para computar la velocidad de comando para alcanzar
   * y determina la velocidad del motor
   */
   calculoVelocidadMedida(Ts, encoderTotal);  //Calcula w_medida a partir de periodo (Ts) y encoder
   volumenPID.Compute();        //Actualiza w_comando en funcion de w_medida y w_setpoint
   
   Serial.print("w_comando: "); Serial.println(w_comando);
   Serial.print("w_medida: "); Serial.println(w_medida);
   Serial.print("w_setpoint: "); Serial.println(w_setpoint);
   
   comandoMotor(dir_motor, pwm_motor, w_comando); //Mueve el motor
}

void tiemposInspExp(int frecuenciaRespiracion, double ratio, double* tiempoInspiratorio, double* tiempoExpiratorio) {
  /**
   * Esta funcion calcula los tiempos inspiratorios y expiratorios para determinar el control del motor
   */
  *tiempoInspiratorio = (60/frecuenciaRespiracion)*ratio; //En segundos
  *tiempoExpiratorio = (60/frecuenciaRespiracion)*(1-ratio); //En segundos
}

void inspiracionVolumen(int frecuenciaRespiracion, double ratio, float volumen) {
  /**
   * Esta funcion calcula la velocidad del motor y lo comanda para control por volumen sin PID
   */
  setpointVelocidadVolumen(frecuenciaRespiracion, ratio, volumen); //Setea velocidad
  comandoMotor(dir_motor, pwm_motor, w_setpoint);
}
