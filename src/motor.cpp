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

//Definicion de motor
MOTOR_t MOTOR;
Motor_States_e motorState;

//Variables para medidas reales
Measured_t measuredData[BUFFER_SIZE];
uint32_t measuredCycleTime = 0;
float measuredTidalVol = 0;
float measuredCompliance = 0;

//Definicion de encoder
Encoder encoder(encoderA, encoderB);

/*Variables de PID*/ 
double Kp_v = 4.8, Ki_v = 1.5, Kd_v = 0.00; //Variables experimentales con nuevo motor
double Kp_p = 4.8, Ki_p = 1.5, Kd_p = 0.00; //ToDo Probar con el sistema entero andando

//PID de control por volumen
PID volumenPID(&MOTOR.wMeasure, &MOTOR.wCommand, &MOTOR.wSetpoint, Kp_v, Ki_v, Kd_v, DIRECT); //Crea objeto PID

//PID de control por presion
PID presionPID(&MOTOR.pMeasure,&MOTOR.pCommand,&MOTOR.pSetpoint, Kp_p, Ki_p, Kd_p, DIRECT); //Crea objeto PID


long startPeriod = 0;
long endPeriod = 0;
long prevPauseTime = 0;


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

  initPID();
  setPinModes();

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

void calculoVelocidadMedida(long Ts) {
  /**
   * Calcula la velocidad angular del periodo dado por el tiempo Ts en ms
   */
  lecturaEncoder();

  MOTOR.wMeasure = MOTOR.encoderCounts*2.0*PI/(encoderCountsPerRev*Ts/SEC_TO_MILLIS); //En rad/s
   
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

  //velocidad = (velocidad < VEL_ANG_MIN)? VEL_ANG_MIN : velocidad;   // Can't go below minimum speed
   
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
  float deltaVelocidad = 0; 
  
  MOTOR.pMeasure = CTRL.pressure;                    //Actualiza la presion medida
  presionPID.Compute();                              //Actualiza p_comando en funcion de p_medida y p_setpoint
  deltaPresion = MOTOR.pCommand - MOTOR.pSetpoint;   //Diferencia de presion para determinar cambio de velocidad
  deltaVelocidad = mapf((float)abs(deltaPresion), PRES_MIN, PRES_MAX, VEL_ANG_MIN, VEL_ANG_MAX);   //Convierte diferencia de presion a diferencia de velocidad (TURBIO)

  /*
  Serial.println("CONSTANTES:");
  Serial.print(Kp_p); Serial.print('\t'); Serial.println(Ki_p);
  */

  #if MOTOR_PID_LOG
    Serial.print("Kp: "); Serial.print(Kp_p); Serial.print('\t');
    Serial.print("Ki: "); Serial.print(Ki_p);
    Serial.println("");

    Serial.print("MOTOR.pCommand: "); Serial.println(MOTOR.pCommand);
    Serial.print("MOTOR.pMeasure: "); Serial.println(MOTOR.pMeasure);
    Serial.print("MOTOR.pSetpoint: "); Serial.println(MOTOR.pSetpoint);
  #endif

  if (deltaPresion >= 0) 
  {
    MOTOR.wCommand += deltaVelocidad;  //Modifica velocidad
  }
  else 
  {
    MOTOR.wCommand -= deltaVelocidad;
  }

  if (MOTOR.wCommand < VEL_ANG_MIN) {
    MOTOR.wCommand = VEL_ANG_MIN;
  }
  else if (MOTOR.wCommand > VEL_ANG_MAX) {
    MOTOR.wCommand = VEL_ANG_MAX;
  }

  /*
  Serial.println("PRESIONES: "); 
  Serial.print(MOTOR.pCommand); Serial.print('\t'); Serial.print(MOTOR.pMeasure); Serial.print('\t'); Serial.println(MOTOR.pSetpoint); Serial.println("");
  
  Serial.println("VELOCIDADES: "); 
  Serial.print(MOTOR.wCommand); Serial.print('\t'); Serial.print(MOTOR.wMeasure); Serial.print('\t'); Serial.println(MOTOR.wSetpoint); Serial.println("");
  */

  comandoMotor(motorDIR, motorPWM, MOTOR.wCommand); //Mueve el motor
}

void controlDeVolumen() {
  /**
   * Esta funcion usa un PID para computar la velocidad de comando para alcanzar
   * y determina la velocidad del motor
   */
  setpointVelocityCalculation();
  calculoVelocidadMedida(MOTOR.Ts);  //Calcula w_medida a partir de periodo (Ts) y encoder
  volumenPID.Compute();        //Actualiza w_comando en funcion de w_medida y w_setpoint
  
  #if MOTOR_PID_LOG
    Serial.print("Kp: "); Serial.print(Kp_v); Serial.print('\t');
    Serial.print("Ki: "); Serial.print(Ki_v);
    Serial.println("");

    Serial.print("MOTOR.wCommand: "); Serial.println(MOTOR.wCommand);
    Serial.print("MOTOR.wMeasure: "); Serial.println(MOTOR.wMeasure);
    Serial.print("MOTOR.wSetpoint: "); Serial.println(MOTOR.wSetpoint);
  #endif
  
  comandoMotor(motorDIR, motorPWM, MOTOR.wCommand); //Mueve el motor
}

void inspiracionVolumen() {
  /**
   * Esta funcion calcula la velocidad del motor y lo comanda para control por volumen sin PID
   */
  setpointVelocityCalculation();                      //Sets maximum speed for inspiration
  comandoMotor(motorDIR, motorPWM, MOTOR.wSetpoint);  //Sends command to move motor

}

void tiemposInspExp() {
  /**
   * Esta funcion calcula los tiempos inspiratorios y expiratorios para determinar el control del motor
   */
  MOTOR.inspirationTime = (MINUTE_MS/MOTOR.breathsMinute)*MOTOR.inspPercentage; //En milisegundos
  MOTOR.expirationTime = (MINUTE_MS/MOTOR.breathsMinute)*(1-MOTOR.inspPercentage); //En milisegundos
  MOTOR.advanceTime = MOTOR.inspirationTime*(1-MOTOR.pausePercentage);  //En milisegundos
  MOTOR.pauseTime = MOTOR.inspirationTime*MOTOR.pausePercentage;
}

void cuentasEncoderVolumen() {
  /**
   * Calcula las cuentas necesarias para desplazar el volumen 
   */
  float recorrido = MOTOR.tidalVolume*ML_TO_MM3/pistonArea;           //Distancia que debe recorrer el piston en mm (*1000 para pasar de ml a mm3)
  float recorridoAngular = recorrido/crownRadius;           //Angulo que debe recorrer el motor en rad
  MOTOR.inspirationCounts = round(encoderCountsPerRev*recorridoAngular/(2*PI));  
}

void Motor_Tasks() {

  MOTOR.limitSwitch = digitalRead(endSwitch);

  checkMotorOvercurrent();  //Check if current has surpassed the limit
  //calculateSystemPeriod();  //Prints in console the system period in microseconds

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


    /*-----------------------RETURN TO HOME-----------------------*/
    case MOTOR_RETURN_HOME_POSITION:
      
      if (MOTOR.motorAction == MOTOR_STOPPED) {                                // Check if it its the first iteration to save time
        MOTOR.motorAction = MOTOR_RETURNING;
        MOTOR.expEndTime = millis() + MOTOR.expirationTime;

        MOTOR.flagInspEnded = true;
      } 

      if ((MOTOR.limitSwitch && (MOTOR.motorAction == MOTOR_STARTING)) || (MOTOR.encoderTotal > 0)) {   // Not in home position conditions
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
      }

      if ((MOTOR.encoderTotal < MOTOR.inspirationCounts + preparationCounts) && (millis() < MOTOR.inspEndTime) && (CTRL.pressure < UI.maxPressure)) {       // Piston moving forward
        // Conditions for inspiration: encoder counts not reached, inspiration time not passed and pressure below max
        
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

        if (CTRL.pressure > UI.maxPressure) {                     // Set alarm if inspiration interrupted by high pressure
          UI_SetAlarm(ALARM_HIGH_PRESSURE);
        }
        else if ((millis() < MOTOR.inspEndTime) && (MOTOR.encoderTotal < minInspirationCounts + preparationCounts)) { // Alarm if motor is not moving during inspiration
          UI_SetAlarm(ALARM_MOTOR_ERROR);
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
      
        measuredTidalVol = calculateDisplacedVolume();          // Calculate how much volume was displaced from encoder
        MOTOR.expirationVolume = measuredTidalVol;    //ToDo tiene que ser medido por sensor de flujo

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
      }

      if ((millis() < MOTOR.inspEndTime) && (MOTOR.encoderTotal < maxVolumeEncoderCounts)) {     // Piston moving forward
        controlDePresion();
        lecturaEncoder();
      }
      else {                                                    // Piston in final position and inspiration time ended
        
        if ((millis() < MOTOR.inspEndTime) && (MOTOR.encoderTotal < minInspirationCounts + preparationCounts)) { // Alarm if motor is not moving during inspiration
          UI_SetAlarm(ALARM_MOTOR_ERROR);
          motorState = MOTOR_POWER_ON;
          break;                       
        }
         
        measuredTidalVol = calculateDisplacedVolume();          // Calculate how much volume was displaced from encoder
        MOTOR.expirationVolume = measuredTidalVol;    //ToDo tiene que ser medido por sensor de flujo

        #if MOTOR_STATES_LOG
          if(MOTOR.encoderTotal > maxVolumeEncoderCounts) {
            Serial.print("Volumen excedido: "); Serial.print(measuredTidalVol); Serial.println("ml");
          }
        #endif

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
          if (millis()-prevPauseTime >= PAUSE_CONTROL_PERIOD) {
            Serial.print("MOTOR.wCommand PAUSE: "); Serial.println(MOTOR.wCommand);
            Serial.print("MOTOR.wDecrement: "); Serial.println(MOTOR.wDecrement);
            MOTOR.wCommand -= MOTOR.wDecrement;                   // Reduces speed
            if (MOTOR.wCommand < VEL_ANG_MIN) {
              MOTOR.wSetpoint = VEL_ANG_MIN;
            } 
            lecturaEncoder();                                     // Get encoder value
            MOTOR.pauseCounts += MOTOR.encoderCounts;             // Save pause counts to return to home correctly
            comandoMotor(motorDIR, motorPWM, MOTOR.wCommand);
            prevPauseTime = millis(); 
          }
        #else
          comandoMotor(motorDIR, motorPWM, VEL_PAUSE); 
        #endif     
      }
      else {                                                    // Piston in final position and inspiration time ended
        comandoMotor(motorDIR, motorPWM, 0);
        Serial.println(MOTOR.pauseCounts);
        
        #if MOTOR_GAP_CORRECTION
          motorState = MOTOR_PREPARE_EXPIRATION;
          #if MOTOR_STATES_LOG
            Serial.println("STATE: PREPARE EXPIRATION");
            Serial.println(motorState);
          #endif
        #else
          motorState = MOTOR_RETURN_HOME_POSITION;
          #if MOTOR_STATES_LOG
            Serial.println("STATE: RETURN TO HOME");
          #endif
        #endif
        
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

void setpointVelocityCalculation() {
  float motorAdvanceTime = MOTOR.inspirationTime*(1-MOTOR.pausePercentage);
  float angularRange = (MOTOR.inspirationCounts*2.0*PI/encoderCountsPerRev)*SEC_TO_MILLIS;
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

  MOTOR.wSetpoint = VEL_ANG_MAX; //DEBUG
  
}

void calculateControlPeriod() {
  MOTOR.tAct = millis();
  MOTOR.Ts = MOTOR.tAct - MOTOR.tPrev;
  MOTOR.tPrev = MOTOR.tAct;
}

void inspirationFirstIteration() {
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
    MOTOR.breathsMinute = UI.breathsMinute;
    MOTOR.tidalVolume = (uint32_t)UI.tidalVolume;
    MOTOR.inspPercentage = ((float)UI.t_i)/100;
    MOTOR.pausePercentage = ((float)UI.t_p)/100;
    MOTOR.pSetpoint = (double)UI.adjustedPressure;
    MOTOR.modeSet = UI.selectedMode;
    MOTOR.pressureTrigger = (CTRL.pressure < CTRL.PEEP + UI.TrP);
  }
}

float calculateDisplacedVolume() {
  float recorridoAngular = (MOTOR.encoderTotal-preparationCounts)*2.0*PI/encoderCountsPerRev;
  float recorrido = recorridoAngular*crownRadius;
  float volumeDisplaced = recorrido*pistonArea;

  /*
  Serial.print("Recorrido: "); Serial.println(recorrido);
  Serial.print("Angulo: "); Serial.println(recorridoAngular);
  Serial.print("Cuentas encoder: "); Serial.println(MOTOR.encoderTotal);
  Serial.print("Volumen calculado: "); Serial.println(volumeDisplaced);
  Serial.println("");
  */

  return volumeDisplaced/ML_TO_MM3;
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

uint8_t getBreathsMinute() {

  uint8_t iterations = 0;
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
    UI_SetAlarm(ALARM_HIGH_BREATHS_PER_MINUTE);
  }
  else if ((iterations < UI.minBreathsMinute) && minutePassed) {
    UI_SetAlarm(ALARM_LOW_BREATHS_PER_MINUTE);
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

  if ((volumeMinute > UI.maxVolumeMinute) && minutePassed) {
    UI_SetAlarm(ALARM_HIGH_VOLUME_PER_MINUTE);
  }
  else if ((volumeMinute < UI.minVolumeMinute) && minutePassed) {
    UI_SetAlarm(ALARM_LOW_VOLUME_PER_MINUTE);
  }

  return volumeMinute/ML_TO_L;

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
    UI_SetAlarm(ALARM_MOTOR_HIGH_CURRENT_CONSUMPTION);
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
    UI_SetAlarm(ALARM_AIR_LEAK);
  }
}

void calculateDecelerationCurve() {
  MOTOR.wDecrement = (MOTOR.wCommand-VEL_ANG_MIN)/(MOTOR.pauseTime/PAUSE_CONTROL_PERIOD);
}