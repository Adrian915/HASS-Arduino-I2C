//=========================   2018, HA PROJECT   =============================//
// Purpose: Basic circuit
//
//=============================================================================//
#include "Alarm.h"

//=============================================================================//
// CONSTRUCTOR AND DESTRUCTOR
//=============================================================================//
Alarm::Alarm(int pinNum)
{	
	Pin = pinNum;
	AlLevlel = NONE;
  thinkLast = 0;
  wasNeeded = true;
  markedForDeletion = false;

  pinMode(pinNum, OUTPUT);
}


//=============================================================================//
// think and other events
//=============================================================================//
void Alarm::Update(unsigned long currentMillis)
{ 

  if (wasNeeded)
  {
    wasNeeded = false;
    LastMillisNeeded = currentMillis;
  }
  else
  { 
  	if (AlLevlel == NONE || markedForDeletion)
  	{
      //shoould I mark it for deletion?
        if (!markedForDeletion && currentMillis - LastMillisNeeded >= LIVE_TIME)
        {
          if (DEBUG_MODE)
          {
             Serial.println("Alarm deleted");
          }
          markedForDeletion = true;
        }
  
        
    		alarmState = LOW;
    		//reset alarms
    		digitalWrite(Pin, alarmState);
    		return;
  	}
  }

	if(currentMillis - thinkLast >= GetBeepInterval()) 
	{
		//update
		thinkLast = currentMillis;
		
		switch (AlLevlel)
		{
			case NONE:
				alarmState = LOW;
				break;
			case LOWALARM:
      case HIGHALARM:
      case INACTIVITYALARM:
        // if the LED is off turn it on and vice-versa:
        if (alarmState == LOW) 
        {
          if (currentMillis - LastBeepTime >= GetBeepDelay())
          {
              alarmState = HIGH;
          }
        } 
        else 
        {
           alarmState = LOW;
           //beep was complete
           LastBeepTime = thinkLast;
        }
        break;
			case URGENTALARM:
				alarmState = HIGH;
				break;
		}

    //Serial.println(alarmState);
		digitalWrite(Pin, alarmState);
	}
}

void Alarm::SetAlarm(AlarmLevel lvl) 
{
	if (lvl != NONE)
	{
		markedForDeletion = false;
	}

  wasNeeded = true;
	AlLevlel = lvl; 
}

void Alarm::StopAlarm() 
{
  AlLevlel = NONE;
  digitalWrite(Pin, LOW);
}

unsigned long Alarm::GetBeepInterval()
{
  switch (AlLevlel)
  {
    case LOWALARM:
    case HIGHALARM:
    case INACTIVITYALARM:
      return BEEP_TIME_INTERVAL;
    case URGENTALARM:
      return 0;
  }
}

unsigned long Alarm::GetBeepDelay()
{
	switch (AlLevlel)
	{
		case LOWALARM:
			return LOW_BEEP_INTERVAL;
		case HIGHALARM:
    case INACTIVITYALARM:
			return HIGH_BEEP_INTERVAL;
	}
}
