//=========================   2014, CAR PROJECT   =============================//
// Purpose: Basic circuit
//
//=============================================================================//
#include "DHTWrapper.h"

//=============================================================================//
// CONSTRUCTOR AND DESTRUCTOR
//=============================================================================//
DHTWrapper::DHTWrapper(uint8_t pin, uint8_t type, uint8_t count) : DHT(pin, type, count)
{
  wasNeeded = true;

  // default to nan
  ValidHumidity = NAN;
  ValidTemperature = NAN;
}

//=============================================================================//
// think and other events
//=============================================================================//
void DHTWrapper::Update(unsigned long currentMillis)
{
	// not needed anymore
	if (markedForDeletion)
		return;

  float lastUpdateTime = currentMillis - thinkLast;

	if(lastUpdateTime > (UPDATE_INTERVAL + MIN_INTERVAL)) 
	{
    //update
    thinkLast = currentMillis;
    
    if (wasNeeded)
    {
      LastMillisNeeded = currentMillis;
      wasNeeded = false;
    }
    else
    {
      if (currentMillis - LastMillisNeeded >= LIVE_TIME)
      {
          markedForDeletion = true;
          if (DEBUG_MODE)
          {
            Serial.println("Flagging DHT to delete");
            Serial.print(GetPin());
            Serial.println();
          }
          return;
      }
    }

    // start new readings; temperature has priority
    float newReadingTemp;
    float newReadingHum;

    // forced reading in celsius
    readValues(&newReadingTemp, &newReadingHum, false, true);

    if (isnan(newReadingTemp) || isnan(newReadingHum))
    {
      faliureCount++;
      if (DEBUG_MODE)
      {
        Serial.println();
        Serial.print("Reading faliure on DHT pin ");
        Serial.print(GetPin());
        Serial.print(" with count of ");
        Serial.print(faliureCount);
        Serial.println();
      }
      
      if (faliureCount >= MAX_FALIURES)
      {
        isAlarm = true;
        if (DEBUG_MODE)
        {
          Serial.println("Catastrophic faliure on DHT");
          Serial.println();
        }
      }

      return;
    }

    // pass ok
    ValidHumidity = newReadingHum;
    ValidTemperature = newReadingTemp;

    // successful reading, reset flags
    faliureCount = 0;
    isAlarm = false;

    if (DEBUG_MODE)
    {
        Serial.println();
        Serial.print("DHT sensor ");
        Serial.print(GetPin());
        Serial.print("has updated with a temperature of ");
        Serial.print(ValidTemperature);
        Serial.print("and humidity of ");
        Serial.print(ValidHumidity);
        Serial.print(" after ");
        Serial.print(lastUpdateTime/1000);
        Serial.print(" seconds");
        Serial.println();
    }
}
}

float DHTWrapper::GetTemperature()
{
  wasNeeded = true;
  markedForDeletion = false;
  
  return ValidTemperature;
}

float DHTWrapper::GetHumidity()
{
  wasNeeded = true;
  markedForDeletion = false;
  
  return ValidHumidity;
}
