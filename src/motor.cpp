/**
 * @file motor.cpp
 * @author Agustin Costa (aguscr95@gmail.com)
 * @brief Motor control for mechanical ventilator
 * @version 0.1
 * @date 2020-04-23
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include "motor.h"
#include "control.h"

//Definicion de motor
MOTOR_t MOTOR;
Motor_States_e motorState;

//Variables para medidas reales
Measured_t measuredData[BUFFER_SIZE];
uint32_t measuredCycleTime = 0;
float measuredTidalVol = 0;

//Definicion de encoder
Encoder encoder(encoderA, encoderB);

/*Variables de PID*/ 
double Kp = 0.35, Ki = 2, Kd = 0.00; //Variables de PID probadas en Rover

//PID de control por volumen
PID volumenPID(&MOTOR.wMeasure, &MOTOR.wCommand, &MOTOR.wSetpoint, Kp, Ki, Kd, DIRECT); //Crea objeto PID

//PID de control por presion
PID presionPID(&MOTOR.pMeasure,&MOTOR.pCommand,&MOTOR.pSetpoint, Kp, Ki, Kd, DIRECT); //Crea objeto PID


void Motor_Init() {
  //Breathing parameters
  MOTOR.breathsMinute = 0;
  MOTOR.volumeMinute = 0;
  MOTOR.inspPercentage = 0;
  MOTOR.modeSet = UI_VOLUME_CONTROL;
  //Angular velocity
  MOTOR.wSetpoint = 0;
  MOTOR.wMeasure = 0;
  MOTOR.wCommand = 0;
  //Pressure
  MOTOR.pSetpoint = 0;
  MOTOR.pMeasure = 0;
  MOTOR.pCommand = 0;
  //Encoder
  MOTOR.encoderCounts = 0;
  MOTOR.encoderTotal = 0;
  MOTOR.inspirationCounts = 0;
  //Times
  MOTOR.inspirationTime = 0;
  MOTOR.expirationTime = 0;
  MOTOR.Ts = 0;
  MOTOR.tAct = 0;
  MOTOR.tPrev = 0;
  MOTOR.inspEndTime = 0;
  MOTOR.expEndTime = 0;
  MOTOR.cycleStart = 0;
  //Motor state
  MOTOR.motorAction = MOTOR_STARTING;
  //Flags
  MOTOR.flagInspEnded = false;
  MOTOR.flagExpEnded = false;
}

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
    presionPID.SetOutputLimits(PRES_MIN, PRES_MAX);

    presionPID.SetSampleTime((1000/(float)CONTROL_SAMPLE_RATE));
  }
  #endif

}

void setPinModes() { 
  /**
   * Setea los pines fisicos, llamar en setup
   */
  pinMode(motorPWM, OUTPUT);
  pinMode(motorDIR, OUTPUT);
  //El seteo de pines de interrupcion lo hace la libreria encoder
  //No hay que setear nada para el analogico
  
}

void lecturaEncoder() {
  /**
   * Utilizando la libreria Encoder se lee el contador de cada rueda y luego se resetea
   * para mantener las cuentas de solo un periodo.
   */
  MOTOR.encoderCounts = encoder.read();

  encoder.write(0);

  MOTOR.encoderTotal += MOTOR.encoderCounts;
}

void calculoVelocidadMedida(long Ts) {
  /**
   * Calcula la velocidad angular del periodo dado por el tiempo Ts en ms
   */
  lecturaEncoder();

  MOTOR.wMeasure = MOTOR.encoderCounts*2.0*PI/(encoderCountsPerRev*Ts/1000); //En rad/s
   
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

void controlDePresion() {
  /**
   * Esta funcion usa un PID para computar la presion de comando para alcanzar 
   * y determina la velocidad del motor para alcanzarla
   */
   double deltaPresion = 0;
   double deltaVelocidad = 0; 
   
   MOTOR.pMeasure = CTRL.pressure;                    //Actualiza la presion medida
   presionPID.Compute();                              //Actualiza p_comando en funcion de p_medida y p_setpoint
   deltaPresion = MOTOR.pSetpoint - MOTOR.pCommand;   //Diferencia de presion para determinar cambio de velocidad
   deltaVelocidad = map(abs(deltaPresion), 0, PRES_MAX, 0, VEL_ANG_MAX);   //Convierte diferencia de presion a diferencia de velocidad (TURBIO)
   
   if (deltaPresion >= 0) 
   {
     MOTOR.wCommand += deltaVelocidad;  //Modifica velocidad
   }
   else 
   {
     MOTOR.wCommand -= deltaVelocidad;
   }

   comandoMotor(motorDIR, motorPWM, MOTOR.wCommand); //Mueve el motor
}

void controlDeVolumen() {
  /**
   * Esta funcion usa un PID para computar la velocidad de comando para alcanzar
   * y determina la velocidad del motor
   */
   calculoVelocidadMedida(MOTOR.Ts);  //Calcula w_medida a partir de periodo (Ts) y encoder
   volumenPID.Compute();        //Actualiza w_comando en funcion de w_medida y w_setpoint
   /*
   Serial.print("MOTOR.wCommand: "); Serial.println(MOTOR.wCommand);
   Serial.print("MOTOR.wMeasure: "); Serial.println(MOTOR.wMeasure);
   Serial.print("MOTOR.wSetpoint: "); Serial.println(MOTOR.wSetpoint);
   */
   comandoMotor(motorDIR, motorPWM, MOTOR.wCommand); //Mueve el motor
}

void inspiracionVolumen() {
  /**
   * Esta funcion calcula la velocidad del motor y lo comanda para control por volumen sin PID
   */
  setpointVelocity(VEL_ANG_MAX);                      //Sets maximum speed for inspiration
  comandoMotor(motorDIR, motorPWM, MOTOR.wSetpoint);  //Sends command to move motor

}

void tiemposInspExp() {
  /**
   * Esta funcion calcula los tiempos inspiratorios y expiratorios para determinar el control del motor
   */
  MOTOR.inspirationTime = (60000.0/MOTOR.breathsMinute)*MOTOR.inspPercentage; //En milisegundos
  MOTOR.expirationTime = (60000.0/MOTOR.breathsMinute)*(1-MOTOR.inspPercentage); //En milisegundos
}

void cuentasEncoderVolumen() {
  /**
   * Calcula las cuentas necesarias para desplazar el volumen 
   */
  float recorrido = MOTOR.tidalVolume*1000/pistonArea;           //Distancia que debe recorrer el piston en mm (*1000 para pasar de ml a mm3)
  float recorridoAngular = recorrido/crownRadius;           //Angulo que debe recorrer el motor en rad
  MOTOR.inspirationCounts = round(encoderCountsPerRev*recorridoAngular/(2*PI));   
}

void Motor_Tasks() {
  switch (motorState)
  {
    
    #if MOTOR_VERBOSE
      Serial.println("STATE: POWER ON");
    #endif
    /*--------------------------POWER ON--------------------------*/
    case MOTOR_POWER_ON:
      MOTOR.motorAction = MOTOR_STARTING;
      motorState = MOTOR_RETURN_HOME_POSITION;

      #if MOTOR_VERBOSE
        Serial.println("STATE: RETURN TO HOME");
      #endif


    /*-----------------------RETURN TO HOME-----------------------*/
    case MOTOR_RETURN_HOME_POSITION:
      
      if (MOTOR.motorAction == MOTOR_STOPPED) {                                // Check if it its the first iteration to save time
        MOTOR.motorAction = MOTOR_RETURNING;
        MOTOR.expEndTime = millis() + MOTOR.expirationTime;

        MOTOR.flagInspEnded = true;

      } 

      if (/*!MOTOR.limitSwitch ||*/ (MOTOR.encoderTotal > 0)) {   // Not in home position conditions
        Motor_ReturnToHomePosition();
        lecturaEncoder();        
      }
      else {                                                  // In home position - either by encoder or limit switch
        comandoMotor(motorDIR, motorPWM, 0);

        if (MOTOR.motorAction == MOTOR_STARTING) {
          MOTOR.encoderTotal = 0;
        } 

        motorState = MOTOR_IDLE;                              // Back to idle state
        
        #if MOTOR_VERBOSE
          Serial.println("STATE: IDLE");
        #endif
      }   
      
      break;

    /*----------------------------IDLE----------------------------*/
    case MOTOR_IDLE:

      Motor_SetBreathingParams();     //First checks if UI.setUpComplete  

      //Serial.print("MOTOR STATE"); Serial.println(MOTOR.motorAction);                      

      if (MOTOR.motorAction == MOTOR_RETURNING || MOTOR.motorAction == MOTOR_WAITING) {             // If expirating
        MOTOR.motorAction = MOTOR_WAITING;                    // Change state variable to waiting
        
        //BEGIN NEW CYCLE
        if (millis() >= MOTOR.expEndTime || MOTOR.pressureTrigger) {    // Inspiration beginning condition

          MOTOR.flagExpEnded = true;

          measuredCycleTime = millis() - MOTOR.cycleStart;    // Calculate last cycle time
          saveRealData(measuredTidalVol, measuredCycleTime);  // Saves cycle time and tidal vol in queue

          MOTOR.cycleStart = millis();                        // Saves time cycle begins

          if (MOTOR.modeSet == UI_VOLUME_CONTROL) {           // Volume mode
            motorState = MOTOR_VOLUME_CONTROL;
            #if MOTOR_VERBOSE
              Serial.println("STATE: VOLUME CONTROL");
            #endif
          }
          else if (MOTOR.modeSet == UI_PRESSURE_CONTROL) {    // Pressure mode
            motorState = MOTOR_PRESSURE_CONTROL;
            #if MOTOR_VERBOSE
              Serial.println("STATE: PRESSURE CONTROL");
            #endif
          }
          else {
            motorState = MOTOR_IDLE;
          }
        }       
      }
      else if (MOTOR.motorAction == MOTOR_STARTING) {           // If starting
        if (UI.setUpComplete) {                                 // Check if setup complete
          //Serial.println("SETUP COMPLETE");
          MOTOR.motorAction = MOTOR_WAITING;
          if (MOTOR.modeSet == UI_VOLUME_CONTROL) {             // Volume mode
            motorState = MOTOR_VOLUME_CONTROL;
            #if MOTOR_VERBOSE
              Serial.println("STATE: VOLUME CONTROL");
            #endif
          }
          else if (MOTOR.modeSet == UI_PRESSURE_CONTROL) {      // Pressure mode
            motorState = MOTOR_PRESSURE_CONTROL;
            #if MOTOR_VERBOSE
              Serial.println("STATE: PRESSURE CONTROL");
            #endif
          }
          else {
            motorState = MOTOR_IDLE;
          }
        }         
      }

      break;

    /*-----------------------VOLUME CONTROL-----------------------*/
    case MOTOR_VOLUME_CONTROL:
      
      if (MOTOR.motorAction == MOTOR_WAITING) {                 // Check if it its the first iteration to save the time
        inspirationFirstIteration();
      }
     
      if (MOTOR.encoderTotal < MOTOR.inspirationCounts) {       // Piston moving forward
        
        #if CONTROL_ACTIVO_VOLUMEN 
        { 
          calculateControlPeriod();   
          controlDeVolumen();         
        }
        #else 
        {
          inspiracionVolumen();
          lecturaEncoder();       
        }
        #endif
      }
      else {
        comandoMotor(motorDIR, motorPWM, 0);

        measuredTidalVol = calculateDisplacedVolume();          // Calculate how much volume was displaced from encoder

        motorState = MOTOR_PAUSE;
        MOTOR.motorAction = MOTOR_STOPPED;

        #if MOTOR_VERBOSE
          Serial.println("STATE: PAUSE");
        #endif
      }

      break;    
      
    /*----------------------PRESSURE CONTROL----------------------*/
    case MOTOR_PRESSURE_CONTROL:
      
      if (MOTOR.motorAction == MOTOR_WAITING) {                               // Check if it its the first iteration to save the time
        inspirationFirstIteration();
      }

      if (millis() < MOTOR.inspEndTime) {                       // Piston moving forward
        controlDePresion();
        lecturaEncoder();
      }
      else {                                                    // Piston in final position and inspiration time ended
        measuredTidalVol = calculateDisplacedVolume();          // Calculate how much volume was displaced from encoder
        motorState = MOTOR_RETURN_HOME_POSITION;    
        MOTOR.motorAction = MOTOR_STOPPED;                      // Set to pause to make RETURN state simpler
        #if MOTOR_VERBOSE
          Serial.println("STATE: RETURN TO HOME");
        #endif
      }

      break;        

    /*--------------------PAUSE IN INSPIRATION--------------------*/
    case MOTOR_PAUSE:

      if (millis() < MOTOR.inspEndTime) {                       // Piston reached final position but time remains in inspiration
        comandoMotor(motorDIR, motorPWM, 0); 
        MOTOR.motorAction = MOTOR_STOPPED;
      }
      else {                                                    // Piston in final position and inspiration time ended
        motorState = MOTOR_RETURN_HOME_POSITION;
        #if MOTOR_VERBOSE
          Serial.println("STATE: RETURN TO HOME");
        #endif
      }

      break;

    /*-------------------------DEFAULT----------------------------*/
    default:
      break;
  }
}

void Motor_ReturnToHomePosition() {
  comandoMotor(motorDIR, motorPWM, -0.8*VEL_ANG_MAX);     // Sets return speed 
}

void setpointVelocity(float vel) {
  MOTOR.wSetpoint = vel;
}

void calculateControlPeriod() {
  MOTOR.tAct = millis();
  MOTOR.Ts = MOTOR.tAct - MOTOR.tPrev;
  MOTOR.tPrev = MOTOR.tAct;
}

void inspirationFirstIteration() {
  MOTOR.motorAction = MOTOR_ADVANCING;
  tiemposInspExp();                                       // Calculates inspiration and expiration times
  MOTOR.inspEndTime = millis() + MOTOR.inspirationTime;   // In miliseconds
  cuentasEncoderVolumen();                                // Calculates MOTOR.inspirationCounts
}

void Motor_SetBreathingParams() {
  if (UI.setUpComplete) {
    MOTOR.breathsMinute = UI.breathsMinute;
    MOTOR.tidalVolume = (uint32_t)UI.tidalVolume;
    MOTOR.inspPercentage =((float)UI.t_i)/100;
    MOTOR.pSetpoint = (double)UI.adjustedPressure;
    MOTOR.modeSet = UI.selectedMode;
  }
}

float calculateDisplacedVolume() {
  float recorridoAngular = MOTOR.inspirationCounts*2*PI/encoderCountsPerRev;
  float recorrido = recorridoAngular*crownRadius;
  float volumeDisplaced = recorrido*pistonArea;

  return volumeDisplaced;
}

void saveRealData(float tidalVol, uint32_t cycleTime) {

  for (size_t i = BUFFER_SIZE-1; i > 0; i--)          //Moves data one place to the right
  {
    measuredData[i] = measuredData[i-1];
  }
  measuredData[0].cycleTime = cycleTime;              //Always saves data in first spot
  measuredData[0].tidalVolume = tidalVol;
}

float getTidalVolume() {
  return measuredTidalVol;
}

uint8_t getBreathsMinute() {

  uint32_t lastCycle = measuredData[0].cycleTime;
  uint8_t iterations = 0;

  for (size_t i = 0; i < BUFFER_SIZE; i++)
  {
    if(measuredData[i].cycleTime < lastCycle-60000) {
      iterations = i;
      break;
    }
  }

  return iterations; 
}

float getVolumeMinute() {

  uint8_t cyclesMinute = getBreathsMinute();
  float volumeMinute = 0;

  for (size_t i = 0; i < cyclesMinute; i++)
  {
    volumeMinute += measuredData[i].tidalVolume;
  }
  
  return volumeMinute;
}

void updateControlVariables() {
  CTRL.breathsMinute = getBreathsMinute();
  CTRL.volume = (uint16_t)getTidalVolume();
  CTRL.volumeMinute = (uint16_t)getVolumeMinute();
}