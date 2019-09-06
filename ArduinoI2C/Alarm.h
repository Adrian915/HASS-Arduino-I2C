#ifndef Alarm_h
#define Alarm_h

#include "SharedDefs.h"

#define LOW_BEEP_INTERVAL 60000
#define HIGH_BEEP_INTERVAL 1000
#define BEEP_TIME_INTERVAL 1000
#define LIVE_TIME 120000

enum AlarmLevel { NONE, LOWALARM, HIGHALARM, URGENTALARM, INACTIVITYALARM };

class Alarm 
{
private:
	int Pin;
	bool markedForDeletion = false;
	int alarmState = LOW; 
	AlarmLevel AlLevlel;
	
	long thinkLast = 0;  // will store last think time
  long LastBeepTime = 0;  // will store last think time

  bool wasNeeded = true;
	volatile unsigned long LastMillisNeeded;
	  
  unsigned long GetBeepInterval();
	unsigned long GetBeepDelay();
        
public:
    Alarm(int pinNum);
	
	int GetPin() {return Pin; };
	
	virtual void SetAlarm(AlarmLevel lvl);
  virtual void StopAlarm();
	virtual AlarmLevel GetAlarmLevel() { return AlLevlel; };
  
	
  virtual void Update(unsigned long currentMillis);

	bool ShouldDelete() { return markedForDeletion; }
	void SetShouldDelete(bool b) { markedForDeletion = b; }
};

#endif
