#ifndef AlarmList_h
#define AlarmList_h

#include "Alarm.h"

class AlarmList
{
	std::vector<Alarm*> Alarms;
	
	Alarm* RegisterAlarm(uint8_t pinNum);
public:
  AlarmList();

  Alarm* GetAlarm(uint8_t pin, bool bCreateNew = true);
	virtual void SetAlarm(uint8_t pin, AlarmLevel lvl);
  virtual void Update(unsigned long currentMillis);
};

#endif
