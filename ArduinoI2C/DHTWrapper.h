#ifndef DHTWrapper_h
#define DHTWrapper_h

#include "SharedDefs.h"
#include "DHT.h"

// keep it alive for half an hour
#define LIVE_TIME 600000
// update every 4 seconds
#define UPDATE_INTERVAL 1000

// needs about 10 faliures every 4 seconds (about one minute of non responsiveness)
#define MAX_FALIURES 10
class DHTWrapper : public DHT
{
private:
	bool markedForDeletion = false;
	bool isAlarm = false;
	
	double ValidHumidity;
	double ValidTemperature;
	
	long thinkLast = 0;        			 // will store last think time
	int faliureCount = 0;

  bool wasNeeded = true;
	volatile unsigned long LastMillisNeeded;
        
public:
    // type 22 is implied
    DHTWrapper(uint8_t pin, uint8_t type = DHT22, uint8_t count = 0);
	
	uint8_t GetPin() {return _pin; };
	
	virtual float GetTemperature();
	virtual float GetHumidity();
	
    virtual void Update(unsigned long currentMillis);
	
	bool IsAlarm() { return isAlarm; };
        
	//deletion
	bool ShouldDelete() { return markedForDeletion; }
	void SetShouldDelete(bool b) { markedForDeletion = b; }
};

#endif
