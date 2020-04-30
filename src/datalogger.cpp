#include "datalogger.h"
#include "control.h"
#include "user_interface.h"
#include "Arduino.h"
#include "config.h"

bool logEnable;
bool printUserSettings;

extern double Kp_v, Ki_v, Kd_v;

uint32_t logTimeoutMillis;

void DataLogger_Init()
{
  Serial.begin(SERIAL_BAUDRATE);

  logEnable = DATALOG_DEFAULT_STATUS;
  printUserSettings = false;

  logTimeoutMillis = DATALOG_STATUS_TIMEOUT;

  // Initial message
  #if DATALOG_PRINT_INIT_MESSAGE
  Serial.print("Respirone. Version ");
  Serial.print(FIRMWARE_VERSION);Serial.print(". ");
  Serial.println(__DATE__);
  #endif
}

void DataLogger_Task()
{
  // Log report
  if (logEnable)
  {
    static uint32_t initialMillis;

    if((millis() - initialMillis) > logTimeoutMillis)
    {
      initialMillis = millis();
      
      DataLogger_ReportStatus();  
    }
  }

  // User settings
  if (printUserSettings)
  {
    printUserSettings = false;
    
    DataLogger_PrintUserSettings();
  }
}

void DataLogger_ReportStatus()
{
  /* Serial.print("BpM="); */  Serial.print((int16_t)(CTRL.breathsMinute));         Serial.print(",");
  /* Serial.print("Pr=");  */  Serial.print((int16_t)(CTRL.pressure));              Serial.print(","); 
  /* Serial.print("Ppk="); */  Serial.print((int16_t)(CTRL.peakPressure));          Serial.print(",");
  /* Serial.print("Ppl="); */  Serial.print((int16_t)(CTRL.plateauPressure));       Serial.print(",");
  /* Serial.print("Ppe="); */  Serial.print((int16_t)(CTRL.PEEP));                  Serial.print(",");
  /* Serial.print("Vr=");  */  Serial.print((int16_t)(CTRL.volume));                Serial.print(",");
  /* Serial.print("VpM="); */  Serial.print((int16_t)(CTRL.volumeMinute));          Serial.print(",");
  /* Serial.print("VpM="); */  Serial.print((int16_t)(CTRL.dynamicCompliance));     Serial.print(",");
  /* Serial.print("VpM="); */  Serial.print((int16_t)(CTRL.currentConsumption));    Serial.println(); 
}

void DataLogger_PrintUserSettings()
{
  switch (UI.selectedMode)
  {
    case UI_VOLUME_CONTROL:
      Serial.print("MS=Vlm,");
  
      Serial.print("BpM="); Serial.print((int16_t)(UI.breathsMinute));     Serial.print(",");
      Serial.print("BMM="); Serial.print((int16_t)(UI.maxBreathsMinute));  Serial.print(","); 
      Serial.print("BMm="); Serial.print((int16_t)(UI.minBreathsMinute));  Serial.print(",");
    
      Serial.print("Ti=");  Serial.print((int16_t)(UI.t_i)); 					     Serial.print(",");
      Serial.print("Tp=");  Serial.print((int16_t)(UI.t_p));  					   Serial.print(",");
      Serial.print("TrP="); Serial.print((int16_t)(UI.TrP));							 Serial.print(",");
    
      Serial.print("Pa=");  Serial.print((int16_t)(UI.adjustedPressure));  Serial.print(",");
      Serial.print("PM=");  Serial.print((int16_t)(UI.maxPressure));       Serial.print(",");
      Serial.print("Pm=");  Serial.print((int16_t)(UI.minPressure));       Serial.print(",");
    
      Serial.print("VT=");  Serial.print((int16_t)(UI.tidalVolume));       Serial.print(",");
      Serial.print("VMM="); Serial.print((int16_t)(UI.maxVolumeMinute));   Serial.print(",");
      Serial.print("VMm="); Serial.print((int16_t)(UI.minVolumeMinute));   Serial.println(); 
      break;

    case UI_PRESSURE_CONTROL:
      Serial.print("<MS=Prs,");
  
      Serial.print("BpM="); Serial.print((int16_t)(UI.breathsMinute));     Serial.print(",");
      Serial.print("BMM="); Serial.print((int16_t)(UI.maxBreathsMinute));  Serial.print(","); 
      Serial.print("BMm="); Serial.print((int16_t)(UI.minBreathsMinute));  Serial.print(",");
    
      Serial.print("Ti=");  Serial.print((int16_t)(UI.t_i)); 					     Serial.print(",");
      Serial.print("Tp=");  Serial.print((int16_t)(UI.t_p));  					   Serial.print(",");
      Serial.print("TrP="); Serial.print((int16_t)(UI.TrP));							 Serial.print(",");
    
      Serial.print("Pa=");  Serial.print((int16_t)(UI.adjustedPressure));  Serial.print(",");
      Serial.print("PM=");  Serial.print((int16_t)(UI.maxPressure));       Serial.print(",");
      Serial.print("Pm=");  Serial.print((int16_t)(UI.minPressure));       Serial.print(",");
    
      Serial.print("VT=");  Serial.print((int16_t)(UI.tidalVolume));       Serial.print(",");
      Serial.print("VMM="); Serial.print((int16_t)(UI.maxVolumeMinute));   Serial.print(",");
      Serial.print("VMm="); Serial.print((int16_t)(UI.minVolumeMinute));   Serial.println(); 
      break;
    
    default:
      // ToDo> error 
      break;
  }
}

void DataLogger_ReportError(uint8_t errorNumber)
{

}

void serialEvent() 
{
  while (Serial.available()) 
  {
    switch ((char)Serial.read())
    {
      // toggle log enable
      case 'l': case 'L':
        logEnable^=true;
        break;

      // print user settings
      case 'u': case 'U':
        printUserSettings=true;  
        break;    

      // increase log period
      case '+':
        logTimeoutMillis += 50; 
        break; 

      // decrease log period
      case '-':
        (logTimeoutMillis>50)? (logTimeoutMillis -= 50) : logTimeoutMillis = 0; 
        break; 


      //DEBUG PID VOLUMEN
      case 'p':
        Kp_v += 0.1;
        break;

      case 'o':
        Kp_v -= 0.1;
        break;

      case 'r':
        Ki_v += 0.1;
        break;
      
      case 'e':
        Ki_v -= 0.1;
        break;
    }
  }
}