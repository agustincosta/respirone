#include "datalogger.h"
#include "control.h"
#include "user_interface.h"
#include "Arduino.h"

bool logEnable;
bool printUserSettings;

uint32_t logTimeoutMillis;

void DataLogger_Init()
{
  Serial.begin(115200);

  logEnable = false;
  printUserSettings = false;

  logTimeoutMillis = DATALOG_STATUS_TIMEOUT;

  // Initial message
  //Serial.println((String)"<Respirone, " + (String)"Version "+String(FIRMWARE_VERSION_HIGH)+"." +String(FIRMWARE_VERSION_LOW) + (String)", "+ (String)__DATE__ + (String)">\n\n");
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
  //Serial.print("<BpM="+String(CTRL.breathsMinute)+",");   
  //Serial.print("Pr="+String(CTRL.pressure)+",");           
  //Serial.print("Ppk="+String(CTRL.peakPressure)+",");      
  //Serial.print("Ppl="+String(CTRL.plateauPressure)+",");   
  //Serial.print("Ppe="+String(CTRL.PEEP)+",");              
  //Serial.print("Vr="+String(CTRL.volume)+",");            
  //Serial.print("VpM="+String(CTRL.volumeMinute)+">\n");  

  Serial.print(String(CTRL.breathsMinute)+",");   
  Serial.print(String(CTRL.pressure)+",");           
  Serial.print(String(CTRL.peakPressure)+",");      
  Serial.print(String(CTRL.plateauPressure)+",");   
  Serial.print(String(CTRL.PEEP)+",");              
  Serial.print(String(CTRL.volume)+",");            
  Serial.print(String(CTRL.volumeMinute)+"\n");   
 
}

void DataLogger_PrintUserSettings()
{
  switch (UI.selectedMode)
  {
    case UI_VOLUME_CONTROL:
      Serial.print("<MS=Vlm,");
      
      Serial.print("IE="+String(UI.i_e)+",");  
  
      Serial.print("BM="+String(UI.breathsMinute)+",");    
      Serial.print("BMM="+String(UI.maxBreathsMinute)+",");      
      Serial.print("BMm="+String(UI.minBreathsMinute)+",");  
      
      Serial.print("PA="+String(UI.adjustedPressure)+",");    
      Serial.print("PM="+String(UI.maxPressure)+",");  
      Serial.print("Pm="+String(UI.minPressure)+",");  
      
      Serial.print("VT="+String(UI.tidalVolume)+",");            
      Serial.print("VMM="+String(UI.maxVolumeMinute)+",");   
      Serial.print("VMm="+String(UI.minVolumeMinute)+",");

      Serial.print("TrP="+String(UI.TrP)+">\n");
      break;

    case UI_PRESSURE_CONTROL:
      Serial.print("<MS=Prs,");

      Serial.print("IE="+String(UI.i_e)+",");  
  
      Serial.print("BM="+String(UI.breathsMinute)+",");    
      Serial.print("BMM="+String(UI.maxBreathsMinute)+",");      
      Serial.print("BMm="+String(UI.minBreathsMinute)+",");  
      
      Serial.print("PA="+String(UI.adjustedPressure)+",");    
      Serial.print("PM="+String(UI.maxPressure)+",");  
      Serial.print("Pm="+String(UI.minPressure)+",");  
      
      Serial.print("VT="+String(UI.tidalVolume)+",");            
      Serial.print("VMM="+String(UI.maxVolumeMinute)+",");   
      Serial.print("VMm="+String(UI.minVolumeMinute)+",");

      Serial.print("TrP="+String(UI.TrP)+">\n");

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
    }
  }
}