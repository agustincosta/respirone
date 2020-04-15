#include "motor_functions.h"
#define logOutput true

int frecuenciaRespiracion = 20; //Respiraciones por minuto
float ratio = 0.3333;           //Relacion I:E (1/I+E)
float volumen = 700;            //Volumen por inspiracion en ml

long encoderCount = 0;
long encoderTotal = 0;

float recorridoAngular = 0;     //Variable que almacena el recorrido angular que debe hacer el motor
long iteraciones = 0;           //Variable para contar cantidad de veces que se ejecuta el loop
double valorPresion = 0;         //Variable para almacerar el valor de presion medido
int presionOffset = 0;          //Cuentas del ADC a presion ambiente para eliminar el offset que da 1V a Pa

void setup() {

  Serial.begin(115200);
  setPinModes();
  volumen = volumen*1000; //A mm3
  presionOffset = analogRead(sensorPresion);

  #if logOutput
    Serial.println("----------- Comienza experimento: Control por volumen -----------");
    Serial.print("Volumen: "); Serial.print(volumen/1000); Serial.println("ml");
    Serial.print("Frecuencia: "); Serial.print(frecuenciaRespiracion); Serial.println("rpm");
    Serial.print("Relacion I:E: 1:"); Serial.println(round(1/ratio)-1);
  #endif
   
}

void loop() {

  #if logOutput
    Serial.print("Iteracion: "); Serial.println(iteraciones);
    iteraciones++;
  #endif
  
  /*----------INSPIRACION-----------*/
  inspiracionVolumen(frecuenciaRespiracion, ratio, volumen, &recorridoAngular); //Setea velocidad
    
  float tiempoInsp = (60000/frecuenciaRespiracion)*ratio;
  long millisInsp = millis(); 

  while (millis()- millisInsp < tiempoInsp) { 
  
    lecturaEncoder(&encoderCount);
    encoderTotal += encoderCount;
    lecturaPresion(&valorPresion, presionOffset);
    
    #if logOutput
      Serial.println(valorPresion);
    #endif
  
  }
  
  /*----------EXPIRACION-----------*/
  inspiracionVolumen(frecuenciaRespiracion, 1-ratio, -volumen, &recorridoAngular); //Setea velocidad

  float tiempoExp = (60000/frecuenciaRespiracion)*(1-ratio);
  long millisExp = millis(); 

  //millis()- millisExp < tiempoExp

  while (encoderTotal > 50) { 
  
    lecturaEncoder(&encoderCount);
    encoderTotal += encoderCount;
    lecturaPresion(&valorPresion, presionOffset);
    
    #if logOutput
      Serial.println(valorPresion);
    #endif  
        
  }
}
