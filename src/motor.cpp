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
#include "alarms.h"
#include "sensor.h"
#include "calibracion.h"

//Definicion de motor
MOTOR_t MOTOR;
Motor_States_e motorState;
Controller_states_e pressureControllerState, volumeControllerState;

//Variables para medidas reales
Measured_t measuredData[BUFFER_SIZE];
uint32_t measuredCycleTime = 0;
float measuredTidalVol = 0;
float measuredCompliance = 0;

//Definicion de encoder
Encoder encoder(encoderB, encoderA);
float selectedVolumeArray[12] = {300, 350, 400, 450, 500, 550, 600, 650, 700, 750, 800, 850};
// long inspirationCountsArray[12] = //{1450, 1650, 1800, 1950, 2100, 2250, 2400, 2600, 2750, 2950, 3100, 3250};
//                                   //{805, 1284, 1474, 1626, 1803, 2005, 2165, 2430, 2613, 2754, 2910, 3160};
//         {1081,
//         1256,
//         1394,
//         1547,
//         1718,
//         1878,
//         2042,
//         2164,
//         2349,
//         2502,
//         2665};      //001


long startPeriod = 0;
long endPeriod = 0;
long prevPauseTime = 0;
long lastEncoder = 0;
float pauseVelFactor = 1;


void Motor_Init() {
  //Breathing parameters
  MOTOR.breathsMinute = 0;
  MOTOR.volumeMinute = 0;
  MOTOR.inspPercentage = 0;
  MOTOR.pausePercentage = 0;
  MOTOR.modeSet = UI_VOLUME_CONTROL;
  //Angular velocity
  MOTOR.wSetpoint = 0;
  MOTOR.wMeasure = 0;
  MOTOR.wCommand = 0;
  MOTOR.wDecrement = 0;
  //Pressure
  MOTOR.pSetpoint = 0;
  MOTOR.pMeasure = 0;
  MOTOR.pCommand = 0;
  MOTOR.adjustedPressureReached = false;
  //Encoder
  MOTOR.encoderCounts = 0;
  MOTOR.encoderTotal = 0;
  MOTOR.inspirationCounts = 0;
  MOTOR.pauseCounts = 0;
  //Times
  MOTOR.inspirationTime = 0;
  MOTOR.expirationTime = 0;
  MOTOR.advanceTime = 0;
  MOTOR.pauseTime = 0;
  MOTOR.Ts = 0;
  MOTOR.tAct = 0;
  MOTOR.tPrev = 0;
  MOTOR.inspEndTime = 0;
  MOTOR.pauseEndTime = 0;
  MOTOR.expEndTime = 0;
  MOTOR.cycleStart = 0;
  //Motor state
  MOTOR.motorAction = MOTOR_STARTING;
  //Flags
  MOTOR.flagInspEnded = false;
  MOTOR.flagExpEnded = false;
  //Sensors
  MOTOR.currentConsumption = 2.0;     //Hardcoded para simular sensor siempre bien
  MOTOR.expirationVolume = 0.0;

  setPinModes();

}

void setPinModes() { 
  /**
   * Setea los pines fisicos, llamar en setup
   */
  pinMode(motorPWM, OUTPUT);
  pinMode(motorDIR, OUTPUT);
  pinMode(endSwitch, INPUT);
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

void comandoMotor(int dirPin, int pwmPin, double velocidad) {
  /**
   * Esta funcion setea un nivel pwm segun la velocidad pasada como variables. Se debe especificar el pin de direccion y de pwm del motor DC.
   * 
   * Dependiendo si la velocidad es negativa o positiva se setea el estado del pin de direccion.
   */

  if(velocidad < 0)
  {
    velocidad = -velocidad ;
    digitalWrite(dirPin, HIGH); //Controla direccion del motor
  }
  else
  {
    digitalWrite(dirPin, LOW);
  }

  //velocidad = (velocidad < VEL_ANG_MIN)? VEL_ANG_MIN : velocidad;   // Can't go below minimum speed
   
  //Serial.print("PWM: "); Serial.println(velocidad/VEL_ANG_MAX*255);
  //Serial.print("DIR: "); Serial.println(direccion);
  analogWrite(pwmPin, velocidad/VEL_ANG_MAX*255);
}

void pressureControlAlgorithm() {
  /**
   * Algoritmo personalizado de control de presion para sustitutir el PID
   * 
   */

  float firstPressureConst = 0.7;         // Pressure value when acceleration finishes and the controller maintains speed
  float secondPressureConst = 0.95;       // Pressure value when acceleration finishes and the controller maintains speed
  float firstVelocityConst = 0.95;         // Percentage of VEL_ANG_MAX that defines the acceleration curve
  float secondVelocityConst = 0.7;        // Percentage of VEL_ANG_MAX that defines the acceleration curve
  float minVelocityConst = 3.0;           // Percentage of VEL_PAUSE when pressure is reached
  float pressureThreshold = 0.95;

  MOTOR.pMeasure = CTRL.pressure;                    //Actualiza la presion medida
  lecturaEncoder(); 

  switch (pressureControllerState)
  {
    case CONTROLLER_FIRST_ACCELERATION:                               // First stage of pressure rise at max speed
      if (MOTOR.pMeasure < firstPressureConst*MOTOR.pSetpoint) {       
        MOTOR.wCommand = firstVelocityConst*VEL_ANG_MAX;
      }
      else {
        pressureControllerState = CONTROLLER_SECOND_ACCELERATION;
      }
      break;
    
    case CONTROLLER_SECOND_ACCELERATION:                              // Second stage of pressure rise at lower speed
      if (MOTOR.pMeasure < secondPressureConst*MOTOR.pSetpoint) {       
        MOTOR.wCommand = secondVelocityConst*VEL_ANG_MAX;
      }
      else {
        pressureControllerState = CONTROLLER_MAINTAIN_SETPOINT;
      }
      break;
    
    case CONTROLLER_MAINTAIN_SETPOINT:                                // Pressure reached and needs to be maintained
      if (MOTOR.pMeasure < pressureThreshold*MOTOR.pSetpoint) {
        //MOTOR.wCommand += (MOTOR.pSetpoint-MOTOR.pMeasure)*0.1;
        //MOTOR.wCommand = (MOTOR.wCommand >= VEL_ANG_MAX)? VEL_ANG_MAX : MOTOR.wCommand;
        MOTOR.wCommand = minVelocityConst*VEL_PAUSE;
      }
      else {
        MOTOR.wCommand = minVelocityConst*VEL_PAUSE;
      }
      break;

    default:
      break;
  }
  
  comandoMotor(motorDIR, motorPWM, MOTOR.wCommand); //Mueve el motor
}

void volumeControlAlgorithm() {
  /**
   * Algoritmo personalizado de control de volumen para sustitutir el PID
   * 
   */

  float firstEncoderConst = 0.7;          // Encoder value when acceleration finishes and the controller maintains speed
  float secondEncoderConst = 0.95;        // Encoder value when acceleration finishes and the controller maintains speed
  float firstVelocityConst = 0.95;        // Percentage of VEL_ANG_MAX that defines the acceleration curve
  float secondVelocityConst = 0.7;        // Percentage of VEL_ANG_MAX that defines the acceleration curve
  float minVelocityConst = 3.0;           // Percentage of VEL_PAUSE when encoder counts are reached
  float countsThreshold = 0.95;

  lecturaEncoder();

  switch (volumeControllerState)
  {
    case CONTROLLER_FIRST_ACCELERATION:                               // First stage of pressure rise at max speed
      if (MOTOR.encoderTotal < firstEncoderConst*MOTOR.inspirationCounts) {       
        MOTOR.wCommand = firstVelocityConst*VEL_ANG_MAX;
      }
      else {
        volumeControllerState = CONTROLLER_SECOND_ACCELERATION;
      }
      break;
    
    case CONTROLLER_SECOND_ACCELERATION:                              // Second stage of pressure rise at lower speed
      if (MOTOR.encoderTotal < secondEncoderConst*MOTOR.inspirationCounts) {       
        MOTOR.wCommand = secondVelocityConst*VEL_ANG_MAX;
      }
      else {
        volumeControllerState = CONTROLLER_MAINTAIN_SETPOINT;
      }
      break;
    
    case CONTROLLER_MAINTAIN_SETPOINT:                                // Pressure reached and needs to be maintained
      if (MOTOR.encoderTotal < countsThreshold*MOTOR.inspirationCounts) {
        //;
      }
      else {
        MOTOR.wCommand = minVelocityConst*VEL_PAUSE;
      }
      break;

    default:
      break;
  }
  
  comandoMotor(motorDIR, motorPWM, MOTOR.wCommand); //Mueve el motor
}

void tiemposInspExp() {
  /**
   * Esta funcion calcula los tiempos inspiratorios y expiratorios para determinar el control del motor
   */
  MOTOR.inspirationTime = (MINUTE_MS/MOTOR.breathsMinute)*MOTOR.inspPercentage; //En milisegundos
  MOTOR.expirationTime = (MINUTE_MS/MOTOR.breathsMinute)*(1-MOTOR.inspPercentage); //En milisegundos
  MOTOR.advanceTime = MOTOR.inspirationTime*(1-MOTOR.pausePercentage);  //En milisegundos
  MOTOR.pauseTime = MOTOR.inspirationTime*MOTOR.pausePercentage;  //En milisegundos
}

void cuentasEncoderVolumen() {
  /**
   * Calcula las cuentas necesarias para desplazar el volumen 
   */
  /*
  float recorrido = MOTOR.tidalVolume*ML_TO_MM3/pistonArea;           //Distancia que debe recorrer el piston en mm (*1000 para pasar de ml a mm3)
  float recorridoAngular = recorrido/crownRadius;           //Angulo que debe recorrer el motor en rad
  MOTOR.inspirationCounts = (long)(encoderCountsPerRev*recorridoAngular/(2*PI)); 
  */
  
  for (size_t i = 0; i < 12; i++)
  {
    if (selectedVolumeArray[i] == MOTOR.tidalVolume) {
      MOTOR.inspirationCounts = inspirationCountsArray[i];
      break;
    }
  }
  
}

//MOTOR TASKS
void Motor_Tasks() {

  MOTOR.limitSwitch = digitalRead(endSwitch);

  // DEBUG - IMPRIME CUENTAS DEL ENCODER
  /*
  int print = (motorState == MOTOR_PAUSE)? 2000:0;
  lecturaEncoder();
  Serial.print(MOTOR.inspirationCounts); Serial.print('\t'); Serial.print(MOTOR.encoderTotal); Serial.print('\t'); Serial.println(print);
  */

  checkMotorOvercurrent();  //Check if current has surpassed the limit
  //calculateSystemPeriod();  //Prints in console the system period in microseconds

  // DEBUG - IMPRIME PRESION PARA PID
  //Serial.print(CTRL.pressure); Serial.print('\t'); Serial.print(MOTOR.pSetpoint); Serial.print('\t'); Serial.print(MOTOR.wCommand); Serial.print('\t'); Serial.println((pressureControllerState+1)*MOTOR.pSetpoint/2);
  

  if (UI.stopVentilation) {
    motorState = MOTOR_POWER_ON;    //Resets state machine to first state
    UI.stopVentilation = false;
  }

  switch (motorState) 
  {
    /*--------------------------POWER ON--------------------------*/
    case MOTOR_POWER_ON:

      #if MOTOR_STATES_LOG
        Serial.println("STATE: POWER ON");
      #endif

      MOTOR.motorAction = MOTOR_STARTING;
      motorState = MOTOR_RETURN_HOME_POSITION;

      #if MOTOR_STATES_LOG
        Serial.println("STATE: RETURN TO HOME");
      #endif

      break;

    /*-----------------------RETURN TO HOME-----------------------*/
    case MOTOR_RETURN_HOME_POSITION:
      
      if (MOTOR.motorAction == MOTOR_STOPPED) {                                // Check if it its the first iteration to save time
        MOTOR.motorAction = MOTOR_RETURNING;
        MOTOR.expEndTime = millis() + MOTOR.expirationTime;

        Serial.println(MOTOR.encoderTotal);

        MOTOR.flagInspEnded = true;
      } 

      if (MOTOR.limitSwitch  /*|| (MOTOR.encoderTotal > 0)*/) {   // Not in home position conditions
        Motor_ReturnToHomePosition();
        lecturaEncoder();      
      }
      else {                                                  // In home position - either by encoder or limit switch 

        comandoMotor(motorDIR, motorPWM, 0);                  // Stops motor for a brief moment before gap correction
        if (MOTOR.motorAction == MOTOR_STARTING) {  
          lecturaEncoder();         
          MOTOR.encoderTotal = 0;
        } 

        #if MOTOR_GAP_CORRECTION
          motorState = MOTOR_PREPARE_INSPIRATION;               // Back to idle state
          #if MOTOR_STATES_LOG
            Serial.println("STATE: PREPARE INSPIRATION");
          #endif
        #else
          motorState = MOTOR_IDLE;
          #if MOTOR_STATES_LOG
            Serial.println("STATE: IDLE");
          #endif
        #endif
      }   
      
      break;

    /*---------------------PREPARE INSPIRATION--------------------*/
    case MOTOR_PREPARE_INSPIRATION:
      if (MOTOR.encoderTotal < preparationCounts) {
        comandoMotor(motorDIR, motorPWM, 0.8*VEL_ANG_MAX);
        lecturaEncoder();
      }
      else {
        comandoMotor(motorDIR, motorPWM, 0);
        delay(100);
        motorState = MOTOR_IDLE;
        #if MOTOR_STATES_LOG
          Serial.println("STATE: IDLE");
        #endif
      }  
      break;

    /*----------------------------IDLE----------------------------*/
    case MOTOR_IDLE:

      Motor_SetBreathingParams();     //First checks if UI.setUpComplete  

      if (MOTOR.motorAction == MOTOR_RETURNING || MOTOR.motorAction == MOTOR_WAITING) {             // If expirating
        MOTOR.motorAction = MOTOR_WAITING;                    // Change state variable to waiting
        
        //BEGIN NEW CYCLE
        MOTOR.pressureTrigger = (CTRL.pressure < CTRL.PEEP + (float)UI.TrP);
        if (millis() >= MOTOR.expEndTime || MOTOR.pressureTrigger) {    // Inspiration beginning condition

          MOTOR.flagExpEnded = true;

          /*End flow measurement integration*/
          compareInspExpVolume();

          //Control variables for UI
          measuredCycleTime = millis() - MOTOR.cycleStart;    // Calculate last cycle time
          measuredCompliance = calculateDynamicCompliance();  // Calculate last cycles dynamic compliance
          saveRealData(measuredTidalVol, measuredCycleTime, measuredCompliance);  // Saves cycle time and tidal vol in queue
          updateControlVariables();

          MOTOR.cycleStart = millis();                        // Saves time cycle begins

          if (MOTOR.modeSet == UI_VOLUME_CONTROL || MOTOR.modeSet == UI_AUTOMATIC_CONTROL) {           // Volume mode
            motorState = MOTOR_VOLUME_CONTROL;
            #if MOTOR_STATES_LOG
              Serial.println("STATE: VOLUME CONTROL");
            #endif
          }
          else if (MOTOR.modeSet == UI_PRESSURE_CONTROL) {    // Pressure mode
            motorState = MOTOR_PRESSURE_CONTROL;
            #if MOTOR_STATES_LOG
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
          Motor_SetBreathingParams();
          UI.setUpComplete = false;

          fillDataBuffer();

          MOTOR.motorAction = MOTOR_WAITING;
          
          if (MOTOR.modeSet == UI_VOLUME_CONTROL  || MOTOR.modeSet == UI_AUTOMATIC_CONTROL) {             // Volume mode
            motorState = MOTOR_VOLUME_CONTROL;
            #if MOTOR_STATES_LOG
              Serial.println("STATE: VOLUME CONTROL");
            #endif
          }
          else if (MOTOR.modeSet == UI_PRESSURE_CONTROL) {      // Pressure mode
            motorState = MOTOR_PRESSURE_CONTROL;
            #if MOTOR_STATES_LOG
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
        volumeControllerState = CONTROLLER_FIRST_ACCELERATION;
      }

      if ((MOTOR.encoderTotal < MOTOR.inspirationCounts*countsFactor + preparationCounts) && (millis() < MOTOR.inspEndTime) && (CTRL.pressure < UI.maxPressure)) {       // Piston moving forward
        // Conditions for inspiration: encoder counts not reached, inspiration time not passed and pressure below max

        volumeControlAlgorithm();    //Motor control based on encoder counts

      }
      else {

        if (CTRL.pressure > UI.maxPressure) {                     // Set alarm if inspiration interrupted by high pressure
          UI_SetMedicalAlarm(ALARM_HIGH_PRESSURE, CTRL.pressure, UI.maxPressure);
        }
        else if ((millis() < MOTOR.inspEndTime) && (MOTOR.encoderTotal < minInspirationCounts + preparationCounts)) { // Alarm if motor is not moving during inspiration
          UI_SetSystemAlarm(ALARM_MOTOR_ERROR);
          motorState = MOTOR_POWER_ON;
          break;                       
        }

        #if MOTOR_STATES_LOG
          if (MOTOR.encoderTotal > MOTOR.inspirationCounts + preparationCounts) {     // Log why motor advance ended
            Serial.println("Cuentas encoder");
          }
          else if (millis() > MOTOR.inspEndTime) {
            Serial.println("Timeout");
          }
          else if (CTRL.pressure > UI.maxPressure) {
            Serial.println("Presion alta");
          }
        #endif

        //comandoMotor(motorDIR, motorPWM, VEL_PAUSE);

        #if MOTOR_PAUSE_DECELERATION 
          motorState = MOTOR_PAUSE;
          #if MOTOR_STATES_LOG
            Serial.println("STATE: PAUSE");
          #endif 
        #else
          motorState = MOTOR_PREPARE_EXPIRATION;
          #if MOTOR_STATES_LOG
            Serial.println("STATE: PREPARE EXPIRATION");
          #endif 
        #endif               
      }
      break;    
      
    /*----------------------PRESSURE CONTROL----------------------*/
    case MOTOR_PRESSURE_CONTROL:
      
      if (MOTOR.motorAction == MOTOR_WAITING) {                               // Check if it its the first iteration to save the time
        inspirationFirstIteration();
        pressureControllerState = CONTROLLER_FIRST_ACCELERATION;
      }

      if ((millis() < MOTOR.inspEndTime) && (MOTOR.encoderTotal < maxVolumeEncoderCounts)) {     // Piston moving forward
        pressureControlAlgorithm();          // Motor control based on pressure sensor
      }
      else {                                                    // Piston in final position and inspiration time ended
        
        if ((millis() > MOTOR.inspEndTime) && (MOTOR.encoderTotal < minInspirationCounts + preparationCounts)) { // Alarm if motor is not moving during inspiration
          UI_SetSystemAlarm(ALARM_MOTOR_ERROR);
          motorState = MOTOR_POWER_ON;
          break;                       
        }

        if (!MOTOR.adjustedPressureReached) {
          UI_SetMedicalAlarm(ALARM_LOW_PRESSURE, CTRL.peakPressure, UI.adjustedPressure);
        }
        else {
          MOTOR.adjustedPressureReached = false;
        }
         
        measuredTidalVol = calculateDisplacedVolume();          // Calculate how much volume was displaced from encoder
        MOTOR.expirationVolume = measuredTidalVol;    //ToDo tiene que ser medido por sensor de flujo

        #if MOTOR_STATES_LOG
          if(MOTOR.encoderTotal > maxVolumeEncoderCounts) {
            Serial.print("Volumen excedido: "); Serial.print(measuredTidalVol); Serial.println("ml");
          }
        #endif

        comandoMotor(motorDIR, motorPWM, VEL_PAUSE);

        motorState = MOTOR_RETURN_HOME_POSITION;    
        MOTOR.motorAction = MOTOR_STOPPED;                      // Set to pause to make RETURN state simpler
        #if MOTOR_STATES_LOG
          Serial.println("STATE: RETURN TO HOME");
        #endif

        /*Start flow measurement integration*/

      }
      break;        

    /*---------------------PREPARE EXPIRATION--------------------*/
    case MOTOR_PREPARE_EXPIRATION:
      if (MOTOR.encoderTotal > MOTOR.inspirationCounts + MOTOR.pauseCounts) {
        comandoMotor(motorDIR, motorPWM, -0.8*VEL_ANG_MAX);
        lecturaEncoder();
      }
      else {
        comandoMotor(motorDIR, motorPWM, 0);

        #if MOTOR_PAUSE_DECELERATION
          motorState = MOTOR_RETURN_HOME_POSITION;
          #if MOTOR_STATES_LOG
            Serial.println("STATE: RETURN TO HOME");
          #endif
        #else
          motorState = MOTOR_PAUSE;
          #if MOTOR_STATES_LOG
            Serial.println("STATE: PAUSE");
          #endif
        #endif  
      }
      break;

    /*--------------------PAUSE IN INSPIRATION--------------------*/
    case MOTOR_PAUSE:

      if(MOTOR.motorAction == MOTOR_ADVANCING) {
        calculateDecelerationCurve();                             // Calculates MOTOR.wDecrement
        MOTOR.motorAction = MOTOR_STOPPED;                        // Changes motor to stopped
        MOTOR.pauseCounts = 0;                                    // Resets pause encoder counts
      }

      if (millis() < MOTOR.pauseEndTime) {                        // Piston reached final position but time remains in inspiration 
        
        #if MOTOR_PAUSE_DECELERATION
          if ((millis()-prevPauseTime >= PAUSE_CONTROL_PERIOD) && (MOTOR.wCommand > 0)) {

            MOTOR.wCommand -= MOTOR.wDecrement;                   // Reduces speed
            if (MOTOR.wCommand < VEL_PAUSE) {
              MOTOR.wCommand = VEL_PAUSE;
            } 
            lecturaEncoder();                                     // Get encoder value
            MOTOR.pauseCounts += MOTOR.encoderCounts;             // Save pause counts to return to home correctly
            
            comandoMotor(motorDIR, motorPWM, MOTOR.wCommand);
            prevPauseTime = millis(); 

            //Serial.print("MOTOR.wCommand: "); Serial.println(MOTOR.wCommand);

          }
        #else
          comandoMotor(motorDIR, motorPWM, 0);
        #endif     
      }
      else {                                                    // Piston in final position and inspiration time ended
        comandoMotor(motorDIR, motorPWM, 0);
        
        measuredTidalVol = calculateDisplacedVolume();          // Calculate how much volume was displaced from encoder
        MOTOR.expirationVolume = measuredTidalVol;              //ToDo tiene que ser medido por sensor de flujo

        motorState = MOTOR_RETURN_HOME_POSITION;
        #if MOTOR_STATES_LOG
          Serial.println("STATE: RETURN TO HOME");
        #endif

        /*
        #if MOTOR_GAP_CORRECTION
          motorState = MOTOR_PREPARE_EXPIRATION;
          #if MOTOR_STATES_LOG
            Serial.println("STATE: PREPARE EXPIRATION");
          #endif
        #else
          motorState = MOTOR_RETURN_HOME_POSITION;
          #if MOTOR_STATES_LOG
            Serial.println("STATE: RETURN TO HOME");
          #endif
        #endif
        */
        
        /*Start flow measurement integration*/

      }
      break;
    
    /*-------------------------DEFAULT----------------------------*/
    default:
      Serial.println("ESTADO DESCONOCIDO");
      motorState = MOTOR_IDLE;
      break;
  }
}

void Motor_ReturnToHomePosition() {
  comandoMotor(motorDIR, motorPWM, -1*VEL_ANG_MAX);     // Sets return speed 
}

void inspirationFirstIteration() {
  lecturaEncoder();
  MOTOR.encoderTotal = 0;
  MOTOR.motorAction = MOTOR_ADVANCING;
  tiemposInspExp();                                       // Calculates inspiration and expiration times
  MOTOR.pauseEndTime = millis() + MOTOR.inspirationTime;  // In miliseconds
  MOTOR.inspEndTime = millis() + MOTOR.advanceTime;       // In miliseconds
  cuentasEncoderVolumen();                                // Calculates MOTOR.inspirationCounts
}

void Motor_SetBreathingParams() {
  if (UI.setUpComplete) {
    if(MOTOR.motorAction != MOTOR_STARTING) {
      UI.setUpComplete = false;
    }
    MOTOR.breathsMinute = (float)UI.breathsMinute;
    MOTOR.tidalVolume = (float)UI.tidalVolume;
    MOTOR.inspPercentage = ((float)UI.t_i)/100;
    MOTOR.pausePercentage = ((float)UI.t_p)/100;
    MOTOR.pSetpoint = (double)UI.adjustedPressure;
    MOTOR.modeSet = UI.selectedMode;
  }
}

float calculateDisplacedVolume() {
  /*
  float recorridoAngular = (MOTOR.encoderTotal-preparationCounts)*2.0*PI/encoderCountsPerRev;
  float recorrido = recorridoAngular*crownRadius;
  float volumeDisplaced = recorrido*pistonArea;

  return volumeDisplaced/ML_TO_MM3;
  */
  float volumeDisplaced;

  for (size_t i = 0; i < 12; i++)
  {
    if (MOTOR.encoderTotal < inspirationCountsArray[i]) {
      volumeDisplaced = mapf((float)MOTOR.encoderTotal, (float)inspirationCountsArray[i-1], (float)inspirationCountsArray[i], selectedVolumeArray[i-1], selectedVolumeArray[i]);
      break;
    }
  }

  return volumeDisplaced;
}

float calculateDynamicCompliance() {
  float deltaP = CTRL.peakPressure - CTRL.PEEP;
  float deltaV = measuredTidalVol;
  return deltaV/deltaP;
}

void saveRealData(float tidalVol, uint32_t cycleTime, float measuredCompliance) {

  for (size_t i = BUFFER_SIZE-1; i > 0; i--)          //Moves data one place to the right
  {
    measuredData[i] = measuredData[i-1];
  }
  measuredData[0].cycleTime = cycleTime;              //Always saves data in first spot
  measuredData[0].tidalVolume = tidalVol;
  measuredData[0].dynamicCompliance = measuredCompliance;
}

float getTidalVolume() {
  return measuredTidalVol;
}

float getBreathsMinute() {

  float iterations = 0;
  uint32_t timeSum = 0;
  bool minutePassed = false;

  for (size_t i = 0; i < BUFFER_SIZE; i++)
  {
    timeSum += measuredData[i].cycleTime;
    
    if(timeSum > 60000) {
      iterations = i+1;
      minutePassed = true;
      break;
    }

  }

  if ((iterations > UI.maxBreathsMinute) && minutePassed) {
    UI_SetMedicalAlarm(ALARM_HIGH_BREATHS_PER_MINUTE, iterations, UI.maxBreathsMinute);
  }
  else if ((iterations < UI.minBreathsMinute) && minutePassed) {
    UI_SetMedicalAlarm(ALARM_LOW_BREATHS_PER_MINUTE, iterations, UI.minBreathsMinute);
  }

  return iterations; 

}

float getVolumeMinute() {

  uint8_t cyclesMinute = getBreathsMinute();
  float volumeMinute = 0;
  bool minutePassed = false;
  uint32_t timeSum = 0;

  for (size_t i = 0; i < cyclesMinute; i++)
  {
    volumeMinute += measuredData[i].tidalVolume;
  }

  //Habria que encontrar una mejor manera de saber si ya paso un minuto
  for (size_t i = 0; i < BUFFER_SIZE; i++)
  {
    timeSum += measuredData[i].cycleTime;
    
    if(timeSum > 60000) {
      minutePassed = true;
      break;
    }
  }

  volumeMinute /= ML_TO_L;

  if ((volumeMinute > UI.maxVolumeMinute) && minutePassed) {
    UI_SetMedicalAlarm(ALARM_HIGH_VOLUME_PER_MINUTE, volumeMinute, UI.maxVolumeMinute);
  }
  else if ((volumeMinute < UI.minVolumeMinute) && minutePassed) {
    UI_SetMedicalAlarm(ALARM_LOW_VOLUME_PER_MINUTE, volumeMinute, UI.minVolumeMinute);
  }

  return volumeMinute;

}

float getDynamicCompliance() {
  return measuredCompliance;
}

void updateControlVariables() {
  CTRL.breathsMinute = getBreathsMinute();
  CTRL.volume = getTidalVolume();
  CTRL.volumeMinute = getVolumeMinute();
  CTRL.dynamicCompliance = getDynamicCompliance();
}

void checkMotorOvercurrent() {
  if(MOTOR.currentConsumption >= maxMotorCurrent) {         
    UI_SetSystemAlarm(ALARM_MOTOR_HIGH_CURRENT_CONSUMPTION);
    motorState = MOTOR_POWER_ON;
  }
}

void calculateSystemPeriod() {
  endPeriod = micros() - startPeriod;
  Serial.print("micros: "); Serial.println(endPeriod);
  startPeriod = micros();
}

void compareInspExpVolume() {
  if (abs(MOTOR.expirationVolume - measuredTidalVol) > AIR_LEAK_THRESHOLD) {
    UI_SetMedicalAlarm(ALARM_AIR_LEAK, MOTOR.expirationVolume-measuredTidalVol, AIR_LEAK_THRESHOLD);
  }
}

void calculateDecelerationCurve() {
  float tiempo = (MOTOR.pauseTime > MAX_PAUSE_DECELERATION_TIME) ? MAX_PAUSE_DECELERATION_TIME : MOTOR.pauseTime;
  MOTOR.wDecrement = (MOTOR.wCommand-VEL_PAUSE)/(tiempo/PAUSE_CONTROL_PERIOD);
}

float minVelPressureFactor() {
  //float pauseVelFactor = mapf(MOTOR.pSetpoint, PRES_MIN, PRES_MAX, 0.6, 2.0);

  return pauseVelFactor;
}

void fillDataBuffer() {
  for (size_t i = BUFFER_SIZE-1; i > 0; i--)          //Moves data one place to the right
  {
    measuredData[i].tidalVolume = MOTOR.tidalVolume;
    measuredData[i].cycleTime = 0;
    measuredData[i].dynamicCompliance = 0;
  }
}