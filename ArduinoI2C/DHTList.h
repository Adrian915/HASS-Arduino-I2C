#ifndef DHTList_h
#define DHTList_h

#include "DHTWrapper.h"

class DHTList
{
	std::vector<DHTWrapper*> DHTWrappers;
	
	DHTWrapper* RegisterDHTWrapper(uint8_t pinNum);
public:
  DHTList();
	
	DHTWrapper* GetDHTWrapper(uint8_t pin);
  virtual void Update(unsigned long currentMillis);
};

#endif
