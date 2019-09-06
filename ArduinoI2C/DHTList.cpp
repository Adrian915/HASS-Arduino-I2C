//=========================   2018, HA PROJECT   =============================//
// Purpose: Basic circuit
//
//=============================================================================//
#include "DHTList.h"

//=============================================================================//
// CONSTRUCTOR AND DESTRUCTOR
//=============================================================================//
DHTList::DHTList()
{	
}


//=============================================================================//
// think and other events
//=============================================================================//
void DHTList::Update(unsigned long currentMillis)
{
   //update all sensors
    for (std::vector<DHTWrapper*>::iterator it = DHTWrappers.begin(); it != DHTWrappers.end(); it++)
    {
        if ((*it)->ShouldDelete())
        {
          delete *it;
          it = DHTWrappers.erase(it)-1;
        }
        else
        {
           (*it)->Update(currentMillis);
        }
    }
}

DHTWrapper* DHTList::GetDHTWrapper(uint8_t pin)
{
   for (std::vector<DHTWrapper*>::iterator it = DHTWrappers.begin(); it != DHTWrappers.end(); it++)
   {
      if ((*it)->GetPin() == pin)
      {
        //Serial.println("Found existing DHT sensor");
        return *it;
      }
   }

   //register new sensor
   return RegisterDHTWrapper(pin);
}

// callback for sending data
DHTWrapper* DHTList::RegisterDHTWrapper(uint8_t pinNum)
{
  DHTWrapper* sensor = new DHTWrapper(pinNum);
  DHTWrappers.push_back(sensor);

  //Serial.println("Created New DHT sensor");
  return sensor;
}
