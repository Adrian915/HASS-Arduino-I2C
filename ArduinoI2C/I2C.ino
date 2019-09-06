
#include <Wire.h>

#include "SharedDefs.h"
#include "DHTList.h"
#include "AlarmList.h"

#define I2C_MSG_IN_SIZE    3
#define I2C_MSG_OUT_SIZE   4

#define CMD_DIGITAL_WRITE  1
#define CMD_DIGITAL_READ   2
#define CMD_ANALOG_WRITE   3
#define CMD_ANALOG_READ    4
#define CMD_SET_PINMODE    5
#define CMD_READ_DHT       6
#define CMD_SET_ALARM      7

#define UNKNOWN_PIN 0xFF

//LED PIN IS DEF ALARM
#define DEFAULT_ALARM_PIN      13
// 30 seconds of inactivity
#define DEFAULT_ALARM_INACTIVITY 30000 

#define SLAVE_ADDRESS 0x04

AlarmList* Alarms;
DHTList* DHTSensors;

unsigned long lastCMDMillis = 0;     

bool newCmdReceived = false;


#define HEAD_TAIL_SIZE 2
#define FLOATS_SENT 2
byte body[HEAD_TAIL_SIZE];

#define FLOATS_SENT 2
float data[FLOATS_SENT];


void setup() 
{
if (DEBUG_MODE)
{
  Serial.begin(9600);
}

// initialize lists
Alarms = new AlarmList();
DHTSensors = new DHTList();

// reset led pin
pinMode(DEFAULT_ALARM_PIN, OUTPUT);
digitalWrite(DEFAULT_ALARM_PIN, LOW);

// initialize i2c as slave
Wire.begin(SLAVE_ADDRESS);

// define callbacks for i2c communication
Wire.onReceive(receiveData);
Wire.onRequest(onRequestEvent);

if (DEBUG_MODE)
{
  Serial.println("Ready!");
}

newCmdReceived = true;
}

void loop() 
{
  unsigned long currentMillis = millis();

  if (newCmdReceived)
  {
    newCmdReceived = false;
    ResetTimeOutAlarm(currentMillis);
  }
  else
  {
    if (currentMillis - lastCMDMillis >= DEFAULT_ALARM_INACTIVITY)
    {
       if (DEBUG_MODE)
        {
          Serial.println("ALARM CREATED BY INACTIVITY TIMEOUT"); 
          Serial.print("time: ");
          Serial.print(currentMillis);
          Serial.print("last cmd reset: ");
          Serial.print(lastCMDMillis);
          Serial.println();
        }
      
      lastCMDMillis = currentMillis;
      Alarms->SetAlarm(DEFAULT_ALARM_PIN, INACTIVITYALARM);
    }
  }
  
  Update(currentMillis);
}

// callback for received data
void receiveData(int byteCount)
{
  //no idea what this is
  if (byteCount == 1)
  {
    Wire.read();
  }
  
  if (byteCount == I2C_MSG_IN_SIZE)
  {
      //reset timeout countdown
      newCmdReceived = true;

      int cmd = Wire.read();
      int pin = Wire.read();
      int value = Wire.read();
    
      //Serial.println(cmd);
      //Serial.println(pin);
      //Serial.println(value);
      if (DEBUG_MODE)
      {
        Serial.println();
        Serial.print("Received command ");
        Serial.print(cmd);
        Serial.print(" on pin ");
        Serial.print(pin);
        Serial.print(" with value ");
        Serial.print(value);
        Serial.println();
      }

      switch(cmd)
      {
      	case CMD_ANALOG_READ:
      	{
    			body[0] = CMD_ANALOG_READ;
    		
    			//populate the send buffer
    			data[0] = analogRead(pin);
    			data[1] = 0;
    			body[1] = 1;
    			break;
      	}
    		case CMD_DIGITAL_READ:
    		{
          body[0] = CMD_DIGITAL_READ;

          //populate the send buffer
          data[0] = digitalRead(pin);
    			data[1] = 0;
    			body[1] = 1;
    			break;
      	}
        case CMD_DIGITAL_WRITE:
        {
          pinMode(pin,OUTPUT);
          digitalWrite(pin,(!value) ? LOW : HIGH);
          break;
        }
        case CMD_READ_DHT:
        {
            body[0] = CMD_READ_DHT;
  
            //populate the send buffer
            DHTWrapper* sensor = DHTSensors->GetDHTWrapper(pin);
            if (sensor && !sensor->IsAlarm())
            {
              data[0] = sensor->GetTemperature();
              data[1] = sensor->GetHumidity();
              body[1] = 1;
            }
            else 
            {
              // TODO: some sort of combo of errors
              body[1] = 0;  // Failed to read from DHT sensor
              return;
            }
            break;
        }
        case CMD_SET_ALARM:
        { 
            //Serial.println("ALARM CREATED BY COMMAND");
            Alarms->SetAlarm(pin, value);
            break;
        }
        default:
        {
           if (DEBUG_MODE)
           {
            Serial.println("Unknown command received");
           }
           
           body[1] = 0;  // Failed to read
            
           break;
        }
      }
  }
}

void Update(unsigned long currentMillis)
{
    // update Alarms
    if (Alarms)
    {
      Alarms->Update(currentMillis);
    }

    // update DHT sensors
    if (DHTSensors)
    {
      DHTSensors->Update(currentMillis);
    }
}

void clearSendData()
{
  // clear body
  for (int b=0; b < HEAD_TAIL_SIZE; b++)
      body[b] = 0;

  // clear data
  for (int d=0; d < FLOATS_SENT; d++)
      data[d] = 0;
}

void onRequestEvent()
{
   Wire.write((byte*) &body, HEAD_TAIL_SIZE*sizeof(byte));
   Wire.write((byte*) &data, FLOATS_SENT*sizeof(float));

   clearSendData();
}

void ResetTimeOutAlarm(unsigned long currentMillis)
{
  //Serial.println("ALARM INACTIVITY RESET"); 
  lastCMDMillis = currentMillis;

  //shut off any existing alarms
  Alarm* alm = Alarms->GetAlarm(DEFAULT_ALARM_PIN, false);
  if (alm && alm->GetAlarmLevel() == INACTIVITYALARM)
  {
    alm->SetAlarm(NONE);
  }
}

