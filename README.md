# HASS-Arduino-I2C
Home Assistent custom component that enables I2C communication with an Arduino. I built this because the pins on my OrangePI Zero board were not enough and I had an Arduino Mega 2560 available.
Supports switches, sensors (digital / analogue read, DHT) and alarms.

Communication is done via `SMBUS2 <https://pypi.org/project/smbus2/>`__ so you will need it set up on your system.

.. code:: bash

    pip install smbus2

Steps to get it working:
1. Set it up in the main configuration file
2. Set up sensors, switches, etc
3. Upload the ArduinoI2C library on your board
4. Connect the I2C. My connection was OPI TWI0_SDA to Arduino SDA and OPI TWI0_SCK to Arduino SCL. Don't forget to connect the ground between the two boards.
5. Connect your devices onto the Arduino pins
6. Power them up

Sensors
---------------------
It can read three types of data: Digital, Analogue and DHT temperature / humidity values. I added the latter because I wanted the Arduino board to handle the data collection and processing of values.

Switches
---------------------
Typical Relays - nothing more to be said.

Alarms
---------------------
At it's core it is simply a switch for a buzzer on the Arduino. It has different levels of alarm annoyance:
-LOWALARM (beeping every once in a while)
-HIGHALARM (constant beeping)
-URGENTALARM (constant tone, I keep this one for impending catastrophic failure)

It also has an inactivity alarm that goes on when your board stops receiving signals from HASS (and turns off when connection is established). I set the default on PIN 13 because that's where the built in LED is.


Have a good one!