//=========================   2018, HA PROJECT   =============================//
// Purpose: Basic circuit
//
//=============================================================================//
#include "AlarmList.h"

//=============================================================================//
// CONSTRUCTOR AND DESTRUCTOR
//=============================================================================//
AlarmList::AlarmList()
{	
}


//=============================================================================//
// think and other events
//=============================================================================//
void AlarmList::Update(unsigned long currentMillis)
{
   //update all alarms
    for (std::vector<Alarm*>::iterator it = Alarms.begin(); it != Alarms.end(); it++)
    {
        if ((*it)->ShouldDelete())
        {
          (*it)->StopAlarm();
          delete *it;
          it = Alarms.erase(it)-1;
        }
        else
        {
           (*it)->Update(currentMillis);
        }
    }
}

void AlarmList::SetAlarm(uint8_t pin, AlarmLevel lvl)
{
	Alarm* alrm = GetAlarm(pin);
    if (alrm)
    {
        alrm->SetAlarm(lvl);
        //Serial.println("Set alarm on pin");
        //Serial.println(pin);
        return;
    }
    if (DEBUG_MODE)
    {
        Serial.println("Failed to set alarm on pin");
        Serial.println(pin);
    }
}

Alarm* AlarmList::GetAlarm(uint8_t pin, bool bCreateNew)
{
   for (std::vector<Alarm*>::iterator it = Alarms.begin(); it != Alarms.end(); it++)
   {
      if ((*it)->GetPin() == pin)
      {
        //Serial.println("Found existing DHT sensor");
        return *it;
      }
   }

   //register new alarm
   if (bCreateNew)
    return RegisterAlarm(pin);
   else
    return NULL;
}

// callback for sending data
Alarm* AlarmList::RegisterAlarm(uint8_t pinNum)
{
  Alarm* newAlarm = new Alarm(pinNum);
  Alarms.push_back(newAlarm);

  //Serial.println("Created New DHT sensor");
  return newAlarm;
}
