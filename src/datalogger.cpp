#include "datalogger.h"
#include "control.h"

void DataLogger_Init()
{
  Serial.begin(115200);
}

void DataLogger_Task()
{
  static uint32_t initialMillis;

  if((millis() - initialMillis) > 200)
  {
    DataLogger_ReportStatus();
    initialMillis = millis();
  }
}

void DataLogger_ReportStatus()
{

}

void DataLogger_ReportError(uint8_t errorNumber)
{

}