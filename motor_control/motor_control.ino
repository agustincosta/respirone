#include "motor_functions.h"

int frecuenciaRespiracion = 15; //Respiraciones por minuto
float ratio = 0.3333;           //Relacion I:E (1/I+E)
float volumen = 500;            //Volumen por inspiracion en ml
long encoderCount = 0;
float recorridoAngular = 0;

void setup() {

  Serial.begin(115200);
  setPinModes();
  volumen = volumen*1000; //A mm3
  //Serial.print("Volumen: "); Serial.println(volumen);
   
}

void loop() {

  /*----------INSPIRACION-----------*/
  fijarSetPointVelocidad(frecuenciaRespiracion, ratio, volumen, &recorridoAngular); //Setea velocidad
  comandoMotor(dir_motor, pwm_motor);
  
  float tiempoInsp = (60000/frecuenciaRespiracion)*ratio;
  long millisInsp = millis(); 

  while (millis()- millisInsp < tiempoInsp) { 
  
    lecturaEncoder(&encoderCount);
  
  }
  Serial.print("Encoder: "); Serial.print(encoderCount); Serial.print(", deberia ser: "); Serial.println(recorridoAngular*cuentasEncoderRevolucion/360);
  
  Serial.println("");
  
  /*----------EXPIRACION-----------*/
  fijarSetPointVelocidad(frecuenciaRespiracion, 1-ratio, -volumen, &recorridoAngular); //Setea velocidad
  comandoMotor(dir_motor, pwm_motor);

  float tiempoExp = (60000/frecuenciaRespiracion)*(1-ratio);
  long millisExp = millis(); 

  //millis()- millisExp < tiempoExp

  while (encoderCount > 50) { 
  
    lecturaEncoder(&encoderCount);  
        
  }
  Serial.print("Encoder: "); Serial.println(encoderCount);
  Serial.println("");
}
