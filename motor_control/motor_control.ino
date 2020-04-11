#include "motor_functions.h"

int frecuenciaRespiracion = 15; //Respiraciones por minuto
float ratio = 0.3333;           //Relacion I:E (1/I+E)
float volumen = 500;            //Volumen por inspiracion en ml
long encoderCounts = 0;         //Variable para almacenar la cuenta del encoder


void setup() {

  Serial.begin(115200);
  setPinModes();
   
}

void loop() {

  fijarSetPointVelocidad(frecuenciaRespiracion, ratio, volumen); //Setea velocidad
  comandoMotor(dir_motor, pwm_motor);
  
  double tiempoInsp = (60000/frecuenciaRespiracion)*ratio;
  long millisInsp = millis(); 

  while (millis()- millisInsp < tiempoInsp) { 
  
    lecturaEncoder(encoderCounts);
    encoderCounts = encoderCounts + 1;
    Serial.println(encoderCounts); 
    Serial.println("Inspirando"); 
  
  }
  
  fijarSetPointVelocidad(frecuenciaRespiracion, 1-ratio, -volumen); //Setea velocidad
  comandoMotor(dir_motor, pwm_motor);

  double tiempoExp = (60000/frecuenciaRespiracion)*(1-ratio);
  long millisExp = millis(); 

  while (millis()- millisExp < tiempoExp) { 
  
    lecturaEncoder(encoderCounts);  
    encoderCounts = encoderCounts - 1;
    Serial.println(encoderCounts);
    Serial.println("Expirando");
    
  }
  
}
