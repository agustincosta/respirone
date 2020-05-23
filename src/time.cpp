//Includes
#include "time.h"
#include <EEPROM.h>
#include "user_interface.h"

// Global variables
time_t total_time, working_time;
Time_states_e timeStates;

void Time_Init ()
{

    Time_Get();

    working_time.hour     = 0;
    working_time.mnt      = 0;
    working_time.scn      = 0;    
    working_time.mscn     = Time_GetMillis ();
    working_time.totalScn = 0;

    //Time_Timer(0);
    timeStates = TIME_IDLE;
}

void Time_Save ()
{
    uint32_t total_sec;
    uint8_t write_byte, csum;

    csum = 0;

    total_sec = total_time.totalScn + working_time.totalScn;
    
    write_byte = (uint8_t)(total_sec & 0x000000FF);
    EEPROM.put(TIME_SCN_ADD+3, write_byte);                 Serial.print(write_byte, HEX); Serial.print(" ");
    csum += write_byte;

    write_byte = (uint8_t)((total_sec & 0x0000FF00)>>8);
    EEPROM.put(TIME_SCN_ADD+2, write_byte);                 Serial.print(write_byte, HEX); Serial.print(" ");
    csum += write_byte;

    write_byte = (uint8_t)((total_sec & 0x00FF0000)>>16);
    EEPROM.put(TIME_SCN_ADD+1, write_byte);                 Serial.print(write_byte, HEX); Serial.print(" ");
    csum += write_byte;

    write_byte = (uint8_t)((total_sec & 0xFF000000)>>24);
    EEPROM.put(TIME_SCN_ADD, write_byte);                   Serial.print(write_byte, HEX); Serial.print(" ");
    csum += write_byte;

    EEPROM.put(TIME_SCN_ADD+4, csum);                       Serial.println(csum, HEX); 
}

void Time_Get ()
{
    uint32_t total_sec;
    uint8_t byte_readed, csum;

    total_sec = 0;
    csum = 0;

    EEPROM.get(TIME_SCN_ADD, byte_readed);
    total_sec = (uint32_t)byte_readed;
    csum += byte_readed;

    EEPROM.get(TIME_SCN_ADD+1, byte_readed);
    total_sec = total_sec << 8 | byte_readed;
    csum += byte_readed;

    EEPROM.get(TIME_SCN_ADD+2, byte_readed);
    total_sec = total_sec << 8 | byte_readed;
    csum += byte_readed;

    EEPROM.get(TIME_SCN_ADD+3, byte_readed);
    total_sec = total_sec << 8 | byte_readed;
    csum += byte_readed;

    EEPROM.get(TIME_SCN_ADD+4, byte_readed);
    if (byte_readed == csum)
    {
        total_time.hour     = total_sec / 3600;
        total_time.mnt      = (total_sec - (3600*total_time.hour))/60;
        total_time.scn      = (total_sec - (3600*total_time.hour) - (60*total_time.mnt));
        total_time.totalScn = total_sec;
    }
    else
    {
        total_time.hour     = 0;
        total_time.mnt      = 0;
        total_time.scn      = 0;
        total_time.totalScn = 0;
        Time_Save();
    }
}

void Time_Update_Task()
{
   if(UI.ventilationOn)
   {
        if((Time_GetMillis() - working_time.mscn) > 0)
        {
            working_time.mscn = working_time.mscn + (Time_GetMillis() - working_time.mscn);

            if (working_time.mscn%MAX_MSCN==0) //overflow milisecond
            {
                working_time.totalScn++;
                working_time.scn++;
                if (working_time.scn>=MAX_SCN) //overflow seconn
                {
                    working_time.scn = 0;
                    working_time.mnt++;
                    if (working_time.mnt>=MAX_MNT) //overflow minute
                    {
                        working_time.mnt = 0;
                        working_time.hour++;
                    }
                }
            }    
        }
        switch(timeStates)
        {
            case TIME_IDLE:
                if(Time_Timer(TIME_TIMEOUT_UPDATE))
                {
                    timeStates = TIME_UPDATE;
                }
            break;

            case TIME_UPDATE:
                Time_Timer(0);
                timeStates = TIME_IDLE;
                Time_Save();
            break;
        }
    }
    else
    {
        Time_Timer(0);
    }
    
}

uint32_t Time_GetMillis ()
{
    return millis();
}

bool Time_Timer(uint32_t n)
{
  static uint32_t initialMillis;

  if(n == 0)
  {
	  initialMillis = millis();
  }
  else if((millis() - initialMillis) > n){
	  return true;
  }
    return false;
}