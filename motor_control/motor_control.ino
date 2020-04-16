#include <EEPROM.h>
#include "motor_functions.h"
#define logOutput true


int frecuenciaRespiracion = 15; //Respiraciones por minuto
float ratio = 0.3333;           //Relacion I:E (1/I+E)
float volumen = 600;            //Volumen por inspiracion en ml

long encoderCount = 0;
long encoderTotal = 0;
long cuentasInspiracion = 0;

float recorridoAngular = 0;     //Variable que almacena el recorrido angular que debe hacer el motor
long iteraciones = 0;           //Variable para contar cantidad de veces que se ejecuta el loop

/*Presiones*/
double valorPresion = 0;         //Variable para almacerar el valor de presion medido
int presionOffset = 0;          //Cuentas del ADC a presion ambiente para eliminar el offset que da 1V a Pa+
double presionMax = 0;

/*Flags*/
bool error_motor = false;       //Condicion de error del motor para frenarlo

/*Tiempos*/
double tiempoInspiratorio = 0;  //Tiempo de inspiracion en segundos
double tiempoExpiratorio = 0;   //Tiempo de expiracion en segundos
long Ts = 0;                    //Periodo de control en milisegundos


void setup() {

  Serial.begin(115200);
  setPinModes();
  initPID();
  volumen = volumen*1000; //A mm3
  
  presionOffset = analogRead(sensorPresion);

  #if logOutput
    Serial.println("----------- Comienza experimento: Control por volumen -----------");
    Serial.print("Volumen: "); Serial.print(volumen/1000); Serial.println("ml");
    Serial.print("Frecuencia: "); Serial.print(frecuenciaRespiracion); Serial.println("rpm");
    Serial.print("Relacion I:E: 1:"); Serial.println(round(1/ratio)-1);
  #endif

  tiemposInspExp(frecuenciaRespiracion, ratio, &tiempoInspiratorio, &tiempoExpiratorio);

  //encoderTotal = EEPROM.get(0,encoderTotal);
  
}

void loop() {

  if (!error_motor) {
    #if logOutput 
    {
      Serial.print("Iteracion: "); Serial.println(iteraciones);
      iteraciones++;
    }
    #endif

    if(encoderTotal <= 20) 
    {
      /*----------INSPIRACION-----------*/
      #if CONTROL_ACTIVO_VOLUMEN
      {
        /*setpointVelocidadVolumen(frecuenciaRespiracion, ratio, volumen);*/ //Setea velocidad
        //comandoMotor(dir_motor, pwm_motor, 1);
        inspiracionVolumen(frecuenciaRespiracion, ratio, volumen);
      }
      #else
      {
        //inspiracionVolumen(frecuenciaRespiracion, ratio, volumen);
        comandoMotor(dir_motor, pwm_motor, VEL_ANG_MAX);
      }
      #endif

      long millisInsp = millis();
      cuentasEncoderVolumen(volumen, &cuentasInspiracion);
      
      float tiempoInsp = (60000/frecuenciaRespiracion)*ratio;
      long tPrev = 0;
      long tAct = 0; 

      while (encoderTotal < cuentasInspiracion) {
        lecturaEncoder(&encoderCount);
        encoderTotal += encoderCount;
        lecturaPresion(&valorPresion, presionOffset);
        
        if (valorPresion > presionMax) {
          presionMax = valorPresion;
        }
        
        #if logOutput
          Serial.print("E "); Serial.println(encoderTotal);
          Serial.print("P "); Serial.println(valorPresion);
        #endif
      }
      comandoMotor(dir_motor, pwm_motor, 0);
    
      while (millis()- millisInsp < tiempoInsp) {
        
        comandoMotor(dir_motor, pwm_motor, 0); 
        tAct = millis();
        
        #if CONTROL_ACTIVO_VOLUMEN 
        {
        /*Control por volumen con PID - Aumento controlado del flujo*/
          Ts = tAct - tPrev;
          tPrev = tAct;
          controlDeVolumen(Ts, &encoderTotal);
          EEPROM.put(0,encoderTotal);
          delay(20);
        } 
        #else 
        {
        /*Control por volumen sin PID - Onda cuadrada de flujo*/
          lecturaEncoder(&encoderCount);
          encoderTotal += encoderCount;
          lecturaPresion(&valorPresion, presionOffset);
          if (valorPresion > presionMax) {
            presionMax = valorPresion;
          }
          EEPROM.put(0,encoderTotal);
          
          #if logOutput
            Serial.print("E "); Serial.println(encoderTotal);
            Serial.print("P "); Serial.println(valorPresion);
          #endif
        } 
        #endif
        
        
        
        if (encoderTotal < 50) {
          //error_motor = true;
          //break;
        }
      }
    }

    //Serial.print("P MAX: "); Serial.println(presionMax);
    
    Serial.println("Inspiracion terminada");
    /*----------EXPIRACION-----------*/
    comandoMotor(dir_motor, pwm_motor, -VEL_ANG_MAX); //Setea velocidad
  
    float tiempoExp = (60000/frecuenciaRespiracion)*(1-ratio);
    long millisExp = millis(); 
  
    //millis()- millisExp < tiempoExp
  
    while (encoderTotal > 0) { 
    
      lecturaEncoder(&encoderCount);
      encoderTotal += encoderCount;
      lecturaPresion(&valorPresion, presionOffset);
      EEPROM.put(0,encoderTotal);
      
      #if logOutput
        Serial.print("E "); Serial.println(encoderTotal);
        Serial.print("P "); Serial.println(valorPresion);
      #endif  
          
    }
    
    if (ratio < 0.5) {
      comandoMotor(dir_motor, pwm_motor, 0);
      delay(1000*(tiempoExpiratorio - tiempoInspiratorio));
      //Serial.println(tiempoExpiratorio);
      //Serial.println(tiempoInspiratorio);
    }
    
  }
}
