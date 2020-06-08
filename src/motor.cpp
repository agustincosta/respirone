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
Watchdog_states_e motorWatchdogState;

//Variables para medidas reales
Measured_t measuredData[BUFFER_SIZE];
uint32_t measuredCycleTime = 0;
float measuredTidalVol = 0;
float measuredCompliance = 0;

//Definicion de encoder
Encoder encoder(encoderB, encoderA);

long startPeriod = 0;
long endPeriod = 0;
long prevPauseTime = 0;
long lastEncoder = 0;
float pauseVelFactor = 1;
float newVelocity = 0;
float velocitySteps = 0;
float pressTransitionIter = 0;


void Motor_Init() {
  //Breathing parameters
  MOTOR.breathsMinute = 0;
  MOTOR.volumeMinute = 0;
  MOTOR.setpointVolume = 0;
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
  MOTOR.pressureModeFirstIteration = true;
  MOTOR.movingBackwards = false;
  MOTOR.movingForwards = false;
  MOTOR.fatalError = false;
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

  float firstPressureConst = 0.6;                   // Pressure value when acceleration finishes and the controller maintains speed
  float secondPressureConst = 0.95;                 // Pressure value when acceleration finishes and the controller maintains speed
  float firstVelocityConst = 0.9;                  // Percentage of VEL_ANG_MAX that defines the acceleration curve
  float secondVelocityConst = 0.5;                  // Percentage of VEL_ANG_MAX that defines the acceleration curve
  float minVelocityConst = minVelPressureFactor();  // Percentage of VEL_PAUSE when pressure is reached
  float pressureThreshold = 0.95;                   // 

  int transitionIterations = 10;                    //
  float transitionTimeConst = 0.2; 

  MOTOR.pMeasure = CTRL.pressure;                    //Actualiza la presion medida
  lecturaEncoder(); 

  switch (pressureControllerState)
  {
    case CONTROLLER_FIRST_ACCELERATION: {                              // First stage of pressure rise at max speed
      MOTOR.movingForwards = true;
      if (MOTOR.pMeasure < firstPressureConst*MOTOR.pSetpoint) {       
        MOTOR.wCommand = firstVelocityConst*VEL_ANG_MAX;
      }
      else {
        #if PRESSURE_CONTROL_TRANSITIONS
          pressureControllerState = CONTROLLER_FIRST_TRANSITION;
        #else
          pressureControllerState = CONTROLLER_SECOND_ACCELERATION;
        #endif
      }
      break;
    }

    case CONTROLLER_FIRST_TRANSITION: {
      pressureControllerState = CONTROLLER_SECOND_ACCELERATION;
      break;
    }

    case CONTROLLER_SECOND_ACCELERATION: {                             // Second stage of pressure rise at lower speed
      MOTOR.movingForwards = true;
      if (MOTOR.pMeasure < secondPressureConst*MOTOR.pSetpoint) {       
        MOTOR.wCommand = secondVelocityConst*VEL_ANG_MAX;
      }
      else {
        #if PRESSURE_CONTROL_TRANSITIONS
          pressureControllerState = CONTROLLER_SECOND_TRANSITION;
          MOTOR_Timer2(0);
          MOTOR.wCommand -= (secondVelocityConst*VEL_ANG_MAX-minVelocityConst*VEL_PAUSE)/transitionIterations;
          pressTransitionIter = 1;
        #else
          pressureControllerState = CONTROLLER_MAINTAIN_SETPOINT;
        #endif
      }
      break;
    }

    case CONTROLLER_SECOND_TRANSITION: {
      if (MOTOR_Timer2(MOTOR.inspirationTime*transitionTimeConst/transitionIterations)) {
        MOTOR.wCommand -= (secondVelocityConst*VEL_ANG_MAX-minVelocityConst*VEL_PAUSE)/transitionIterations;
        pressTransitionIter++;
        MOTOR_Timer2(0);
      }
      if (pressTransitionIter == transitionIterations) {
        pressureControllerState = CONTROLLER_MAINTAIN_SETPOINT;
      }
      break;
    }

    case CONTROLLER_MAINTAIN_SETPOINT: {                                // Pressure reached and needs to be maintained
      MOTOR.movingForwards = false;
      if (MOTOR.pMeasure < pressureThreshold*MOTOR.pSetpoint) {
        //MOTOR.wCommand = mapf(MOTOR.pMeasure - pressureThreshold, 0, 20, minVelocityConst*VEL_PAUSE, VEL_ANG_MAX);
        //MOTOR.wCommand = minVelocityConst*VEL_PAUSE;
      }
      else {
        MOTOR.wCommand = minVelocityConst*VEL_PAUSE;
      }
      break;
    }
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

  float firstVelocityConst = 1.10;          // Percentage of VEL_ANG_MAX that defines the acceleration curve
  float secondVelocityConst = 1.10;         // Percentage of VEL_ANG_MAX that defines the acceleration curve
  float minVelocityConst = 3.0;             // Percentage of VEL_PAUSE when encoder counts are reached

  float firstAdvanceConst = 0.50;           // Encoder value when acceleration finishes and the controller maintains speed
  float secondAdvanceConst = 0.60;          // Encoder value when acceleration finishes and the controller maintains speed
  
  long firstTransitionCounts = 100;
  long secondTransitionCounts = 50;
  int transitionIterations = 10;

  float advanceThreshold = 1.00;

  #if VOLUME_FLOW_CONTROL
    float controlVariable = CTRL.volume;
    float setpointVariable = MOTOR.tidalVolume;
  #else
    long controlVariable = MOTOR.encoderTotal;
    long setpointVariable = MOTOR.inspirationCounts-100;
  #endif
  

  lecturaEncoder();

  switch (volumeControllerState)
  {
    case CONTROLLER_FIRST_ACCELERATION: {                              // First stage of pressure rise at max speed
      MOTOR.movingForwards = true;
      if (controlVariable < setpointVariable) {       
        MOTOR.wCommand = firstVelocityConst*MOTOR.wSetpoint;
      }
      else {
        #if VOLUME_CONTROL_TRANSITIONS
          volumeControllerState = CONTROLLER_FIRST_TRANSITION;
        #else
          volumeControllerState = CONTROLLER_SECOND_ACCELERATION;
        #endif
        MOTOR.calculateDynamicSpeed = true;
      }
      break;
    }

    case CONTROLLER_FIRST_TRANSITION: {

      MOTOR.movingForwards = true;

      if (MOTOR.calculateDynamicSpeed) {                              // Dynamic speed calculation for second part
        float nextVel = (setpointVariable-firstAdvanceConst*setpointVariable + firstTransitionCounts)*SEC_TO_MILLIS/(MOTOR.inspEndTime - millis());
        float actVel = firstVelocityConst*MOTOR.wSetpoint;
        velocitySteps = (actVel - nextVel)/transitionIterations;
        MOTOR.calculateDynamicSpeed = false;
      }

      if (controlVariable < firstAdvanceConst*setpointVariable + firstTransitionCounts) {
        int i = 1;
        if ((i <= transitionIterations) && (controlVariable < firstAdvanceConst*setpointVariable + i*firstTransitionCounts/transitionIterations)) {
          MOTOR.wCommand -= velocitySteps;
          i++;
        }
      }
      else {
        volumeControllerState = CONTROLLER_SECOND_ACCELERATION;
        MOTOR.calculateDynamicSpeed = true;
      }
      break;
    }

    case CONTROLLER_SECOND_ACCELERATION: {                             // Second stage of pressure rise at lower speed
      MOTOR.movingForwards = true;

      if (MOTOR.calculateDynamicSpeed) {                              // Dynamic speed calculation for second part
        newVelocity = (setpointVariable-controlVariable)*SEC_TO_MILLIS/(MOTOR.inspEndTime - millis());
        MOTOR.calculateDynamicSpeed = false;
      }

      if (controlVariable < secondAdvanceConst*setpointVariable) {       
        MOTOR.wCommand = secondVelocityConst*newVelocity;
      }
      else {
        #if VOLUME_CONTROL_TRANSITIONS
          volumeControllerState = CONTROLLER_SECOND_TRANSITION;
        #else
          volumeControllerState = CONTROLLER_MAINTAIN_SETPOINT;
        #endif
        MOTOR.calculateDynamicSpeed = true;
      }
      break;
    }

    case CONTROLLER_SECOND_TRANSITION: {
      if (MOTOR.calculateDynamicSpeed) {                              // Dynamic speed calculation for second part
        float nextVel = minVelocityConst*VEL_PAUSE;
        float actVel = secondVelocityConst*newVelocity;
        velocitySteps = (actVel - nextVel)/transitionIterations;
        MOTOR.calculateDynamicSpeed = false;
      }

      if (controlVariable < secondAdvanceConst*setpointVariable + secondTransitionCounts) {
        int i = 1;
        if ((i <= transitionIterations) && (controlVariable < secondAdvanceConst*setpointVariable + i*secondTransitionCounts/transitionIterations)) {
          MOTOR.wCommand -= velocitySteps;
          i++;
        }
      }
      else {
        volumeControllerState = CONTROLLER_MAINTAIN_SETPOINT;
      }

      break;
    }

    case CONTROLLER_MAINTAIN_SETPOINT: {                               // Pressure reached and needs to be maintained
      MOTOR.movingForwards = false;
      if (controlVariable < advanceThreshold*setpointVariable) {
        //;
      }
      else {
        MOTOR.wCommand = minVelocityConst*VEL_PAUSE;
      }
      break;
    }

    default: {
      break;
    }
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
    if (selectedVolumeArray[i] > MOTOR.setpointVolume) {
      float newCounts = mapf(MOTOR.setpointVolume, selectedVolumeArray[i-1], selectedVolumeArray[i], (float)inspirationCountsArray[i-1], (float)inspirationCountsArray[i]);
      MOTOR.inspirationCounts = (long)newCounts;
      break;
    }
  }
}

//MOTOR TASKS
void Motor_Tasks() {

  MOTOR.limitSwitch = digitalRead(endSwitch);

  checkMotorBlocked();

  // DEBUG - IMPRIME CUENTAS DEL ENCODER
  /*
  int pauseState = (motorState == MOTOR_PAUSE)? 2000:0;
  lecturaEncoder();
  Serial.print(MOTOR.inspirationCounts); Serial.print('\t'); Serial.print(MOTOR.encoderTotal); Serial.print('\t'); Serial.print(CTRL.pressure); Serial.print('\t'); Serial.println(pauseState);
  */
  //calculateSystemPeriod();  //Prints in console the system period in microseconds

  // DEBUG - IMPRIME PRESION PARA PID
  //Serial.print(CTRL.pressure); Serial.print('\t'); Serial.print(MOTOR.pSetpoint); Serial.print('\t'); Serial.print(MOTOR.wCommand/100); Serial.print('\t'); Serial.println((pressureControllerState+1)*MOTOR.pSetpoint/2);
  
  if (MOTOR.fatalError) {
    motorState = MOTOR_ERROR;
    MOTOR.fatalError = false;
    #if MOTOR_STATES_LOG
      Serial.println("MOTOR ERROR");
    #endif
  }

  if (UI.stopVentilation) {
    motorState = MOTOR_POWER_ON;    //Resets state machine to first state
    UI.stopVentilation = false;
  }

  switch (motorState) 
  {
    /*--------------------------POWER ON--------------------------*/
    case MOTOR_POWER_ON: {

      #if MOTOR_STATES_LOG
        Serial.println("STATE: POWER ON");
      #endif

      MOTOR.motorAction = MOTOR_STARTING;
      motorState = MOTOR_RETURN_HOME_POSITION;

      #if MOTOR_STATES_LOG
        Serial.println("STATE: RETURN TO HOME");
      #endif

      break;
    }
    /*-----------------------RETURN TO HOME-----------------------*/
    case MOTOR_RETURN_HOME_POSITION: {
      
      if (MOTOR.motorAction == MOTOR_STOPPED) {                                // Check if it its the first iteration to save time
        MOTOR.motorAction = MOTOR_RETURNING;
        MOTOR.expEndTime = millis() + MOTOR.expirationTime;

        #if CALIBRACION
          Serial.println(MOTOR.encoderTotal);
        #endif

        MOTOR.flagInspEnded = true;
      } 

      if ((MOTOR.limitSwitch)) {   // Not in home position conditions
        MOTOR.movingBackwards = true;
        Motor_ReturnToHomePosition();
        lecturaEncoder();      
      }
      else {                                                  // In home position - either by encoder or limit switch 
        MOTOR.movingBackwards = false;
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
    }
    /*---------------------PREPARE INSPIRATION--------------------*/
    case MOTOR_PREPARE_INSPIRATION: {
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
    }
    /*----------------------------IDLE----------------------------*/
    case MOTOR_IDLE: {

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
            if (MOTOR.pressureModeFirstIteration) {
              motorState = MOTOR_VOLUME_CONTROL;
              MOTOR.tidalVolume = 500;
            }
            else {
              motorState = MOTOR_PRESSURE_CONTROL;
            }
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
          measuredTidalVol = MOTOR.tidalVolume;
          fillDataBuffer();

          MOTOR.motorAction = MOTOR_WAITING;
          
          if (MOTOR.modeSet == UI_VOLUME_CONTROL  || MOTOR.modeSet == UI_AUTOMATIC_CONTROL) {             // Volume mode
            MOTOR.cycleStart = millis();
            motorState = MOTOR_VOLUME_CONTROL;
            #if MOTOR_STATES_LOG
              Serial.println("STATE: VOLUME CONTROL");
            #endif
          }
          else if (MOTOR.modeSet == UI_PRESSURE_CONTROL) {      // Pressure mode
            MOTOR.cycleStart = millis();
            if (MOTOR.pressureModeFirstIteration) {
              motorState = MOTOR_VOLUME_CONTROL;
              MOTOR.tidalVolume = 500;                          // Volume for the first iteration of pressure mode
            }
            else {
              motorState = MOTOR_PRESSURE_CONTROL;
            }
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
    }
    /*-----------------------VOLUME CONTROL-----------------------*/
    case MOTOR_VOLUME_CONTROL: {

      if (MOTOR.motorAction == MOTOR_WAITING) {                 // Check if it its the first iteration to save the time
        inspirationFirstIteration();
        setpointVelocityCalculation();
        volumeControllerState = CONTROLLER_FIRST_ACCELERATION;
      }

      if ((MOTOR.encoderTotal < MOTOR.inspirationCounts*countsFactor + preparationCounts) && (millis() < MOTOR.inspEndTime) && (CTRL.pressure < UI.maxPressure)) {       // Piston moving forward
        // Conditions for inspiration: encoder counts not reached, inspiration time not passed and pressure below max
        
        volumeControlAlgorithm();    //Motor control based on encoder counts

      }
      else {
        MOTOR.movingForwards = false;
        if (CTRL.pressure > UI.maxPressure) {                     // Set alarm if inspiration interrupted by high pressure
          UI_SetMedicalAlarm(ALARM_HIGH_PRESSURE, CTRL.pressure, UI.maxPressure);
        }
        else if ((millis() < MOTOR.inspEndTime) && (MOTOR.encoderTotal < minInspirationCounts + preparationCounts)) { // Alarm if motor is not moving during inspiration
          UI_SetSystemAlarm(ALARM_MOTOR_ERROR);
          motorState = MOTOR_ERROR;
          Serial.println("ERROR");
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

        MOTOR.pressureModeFirstIteration = false;

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
    } 
    /*----------------------PRESSURE CONTROL----------------------*/
    case MOTOR_PRESSURE_CONTROL: {
      
      if (MOTOR.motorAction == MOTOR_WAITING) {                               // Check if it its the first iteration to save the time
        inspirationFirstIteration();
        pressureControllerState = CONTROLLER_FIRST_ACCELERATION;
        MOTOR.pSetpoint = CTRL.PEEP + (float)UI.adjustedPressure;             // Set pressure is referentiated to the PEEP
      }

      if ((millis() < MOTOR.inspEndTime) && (MOTOR.encoderTotal < maxVolumeEncoderCounts)) {     // Piston moving forward
        pressureControlAlgorithm();          // Motor control based on pressure sensor
        if (CTRL.pressure >= MOTOR.pSetpoint - 1) {
          MOTOR.adjustedPressureReached = true;
        }
      }
      else {                                                    // Piston in final position and inspiration time ended
        
        if ((millis() > MOTOR.inspEndTime) && (MOTOR.encoderTotal < minInspirationCounts + preparationCounts)) { // Alarm if motor is not moving during inspiration
          UI_SetSystemAlarm(ALARM_MOTOR_ERROR);
          motorState = MOTOR_ERROR;
          Serial.println("ERROR");
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
    }
    /*---------------------PREPARE EXPIRATION--------------------*/
    case MOTOR_PREPARE_EXPIRATION: {
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
    }
    /*--------------------PAUSE IN INSPIRATION--------------------*/
    case MOTOR_PAUSE: {

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
    }
    /*-------------------------DEFAULT----------------------------*/
    case MOTOR_ERROR: {
      comandoMotor(motorDIR,motorPWM, 0);
      break;
    }
    /*-------------------------DEFAULT----------------------------*/
    default: {
      Serial.println("ESTADO DESCONOCIDO");
      motorState = MOTOR_IDLE;
      break;
    }
  }
}

void Motor_ReturnToHomePosition() {
  if (MOTOR.motorAction != MOTOR_STARTING) {
    if (MOTOR.encoderTotal > returnDecelerationCounts) {
      MOTOR.wCommand = -1*VEL_ANG_MAX;
    }
    else {
      #if COUNTS_SECOND_SPEEDS
        MOTOR.wCommand += 135;
      #else
        MOTOR.wCommand += 0.05;
      #endif
      MOTOR.wCommand = (MOTOR.wCommand > -VEL_ANG_MIN)? -VEL_ANG_MIN : MOTOR.wCommand;
    }
  }
  else {
    MOTOR.wCommand = -1*VEL_ANG_MAX;
  }

  comandoMotor(motorDIR, motorPWM, MOTOR.wCommand);     // Sets return speed 
}

void inspirationFirstIteration() {
  lecturaEncoder();
  MOTOR.encoderTotal = 0;
  MOTOR.motorAction = MOTOR_ADVANCING;
  tiemposInspExp();                                       // Calculates inspiration and expiration times
  MOTOR.pauseEndTime = millis() + MOTOR.inspirationTime;  // In miliseconds
  MOTOR.inspEndTime = millis() + MOTOR.advanceTime;       // In miliseconds
  MOTOR.setpointVolume = MOTOR.tidalVolume + (MOTOR.tidalVolume-measuredTidalVol)*VOLUME_COMPENSATION_CONST;
  cuentasEncoderVolumen();                                // Calculates MOTOR.inspirationCounts
}

void Motor_SetBreathingParams() {
  if (UI.setUpComplete) {
    if(MOTOR.motorAction != MOTOR_STARTING) {
      UI.setUpComplete = false;
    }
    MOTOR.breathsMinute = (float)UI.breathsMinute;
    MOTOR.tidalVolume = (float)UI.tidalVolume;
    //measuredTidalVol = MOTOR.tidalVolume + (MOTOR.tidalVolume-measuredTidalVol)/2;
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

  //Serial.print(timeSum); Serial.print('\t'); Serial.print(minutePassed); Serial.print('\t'); Serial.println(iterations);

  if ((iterations > UI.maxBreathsMinute) && minutePassed) {
    UI_SetMedicalAlarm(ALARM_HIGH_BREATHS_PER_MINUTE, iterations, UI.maxBreathsMinute);
    Serial.println("HIGH RPM");
  }
  else if ((iterations < UI.minBreathsMinute) && minutePassed) {
    UI_SetMedicalAlarm(ALARM_LOW_BREATHS_PER_MINUTE, iterations, UI.minBreathsMinute);
    Serial.println("LOW RPM");
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

  //Serial.print(volumeMinute); Serial.print('\t'); Serial.print(minutePassed); Serial.print('\t'); Serial.println(timeSum);

  if ((volumeMinute > UI.maxVolumeMinute) && minutePassed) {
    UI_SetMedicalAlarm(ALARM_HIGH_VOLUME_PER_MINUTE, volumeMinute, UI.maxVolumeMinute);
    Serial.println("HIGH VM");
  }
  else if ((volumeMinute < UI.minVolumeMinute) && minutePassed) {
    UI_SetMedicalAlarm(ALARM_LOW_VOLUME_PER_MINUTE, volumeMinute, UI.minVolumeMinute);
    Serial.println("LOW VM");
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
    motorState = MOTOR_ERROR;
    Serial.println("ERROR");
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

  float pauseVelFactor = mapf(MOTOR.pSetpoint, 15, 35, 0.9, 1.7);
  return pauseVelFactor;
}

void fillDataBuffer() {
  for (size_t i = 0; i < BUFFER_SIZE; i++)          //Moves data one place to the right
  {
    measuredData[i].tidalVolume = MOTOR.tidalVolume;
    measuredData[i].cycleTime = MINUTE_MS/MOTOR.breathsMinute;
    measuredData[i].dynamicCompliance = 0;
  }
}

void setpointVelocityCalculation() {
  float motorAdvanceTime = MOTOR.inspirationTime*(1-MOTOR.pausePercentage)/SEC_TO_MILLIS;
  #if COUNTS_SECOND_SPEEDS
    float angularRange = MOTOR.inspirationCounts;
  #else
    float angularRange = (MOTOR.inspirationCounts*TWO_PI/encoderCountsPerRev)*1.9;
  #endif
  
  float angularVelocity = angularRange/motorAdvanceTime;

  if (angularVelocity < VEL_ANG_MIN) {
    MOTOR.wSetpoint = VEL_ANG_MIN;
  }
  else if (angularVelocity > VEL_ANG_MAX) {
    MOTOR.wSetpoint = VEL_ANG_MAX;
  }
  else {
    MOTOR.wSetpoint = angularVelocity;
  }
}

bool MOTOR_Timer(uint32_t n) {
  static uint32_t initialMotorMillis;

  if(n == 0)
  {
	  initialMotorMillis = millis();
  }
  else if((millis() - initialMotorMillis) > n){
	  return true;
  }
    return false;
}

bool MOTOR_Timer2(uint32_t n) {
  static uint32_t initialMotorMillis;

  if(n == 0)
  {
	  initialMotorMillis = millis();
  }
  else if((millis() - initialMotorMillis) > n){
	  return true;
  }
    return false;
}

void checkMotorBlocked() {

  switch (motorWatchdogState)
  {
  case WATCHDOG_IDLE:
    if (MOTOR.movingForwards) {
      motorWatchdogState = WATCHDOG_MOVE_FORWARDS;
      MOTOR_Timer(0);
    }
    else if (MOTOR.movingBackwards) {
      motorWatchdogState = WATCHDOG_MOVE_BACKWARDS;
      MOTOR_Timer(0);
    }
    break;
  
  case WATCHDOG_MOVE_FORWARDS:
    if (MOTOR.movingForwards) {
      if (MOTOR.encoderTotal <= MOTOR.prevCounts) {
        if (MOTOR_Timer(TIMEOUT_MOTOR_WATCHDOG)) {
          motorWatchdogState = WATCHDOG_ERROR;
          MOTOR.fatalError = true;
          UI_SetSystemAlarm(ALARM_MOTOR_ERROR);
        }
      }
      else {
        MOTOR_Timer(0);
        MOTOR.prevCounts = MOTOR.encoderTotal;
      }
    }
    else {
      motorWatchdogState = WATCHDOG_IDLE;
    }
    break;
  
  case WATCHDOG_MOVE_BACKWARDS:
    if (MOTOR.movingBackwards) {
      if (MOTOR.encoderTotal >= MOTOR.prevCounts) {
        if (MOTOR_Timer(TIMEOUT_MOTOR_WATCHDOG)) {
          motorWatchdogState = WATCHDOG_ERROR;
          MOTOR.fatalError = true;
          UI_SetSystemAlarm(ALARM_MOTOR_ERROR);
        }
      }
      else {
        MOTOR_Timer(0);
        MOTOR.prevCounts = MOTOR.encoderTotal;
      }
    }
    else {
      motorWatchdogState = WATCHDOG_IDLE;
    }
    break;
  
  case WATCHDOG_ERROR:
    break;
  
  default:
    motorWatchdogState = WATCHDOG_IDLE;
    break;
  }
}



