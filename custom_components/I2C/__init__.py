"""
Support for Arduino boards running with the Firmata firmware.

For more details about this component, please refer to the documentation at
https://home-assistant.io/components/arduino/
"""
import logging
import struct
import smbus2
import time
import voluptuous as vol

from homeassistant.const import (
    EVENT_HOMEASSISTANT_START, EVENT_HOMEASSISTANT_STOP)
from homeassistant.const import CONF_PORT
import homeassistant.helpers.config_validation as cv

CMD_DIGITAL_WRITE = 1
CMD_DIGITAL_READ  = 2
CMD_ANALOG_WRITE  = 3
CMD_ANALOG_READ   = 4
CMD_SET_PINMODE   = 5
CMD_READ_DHT      = 6
CMD_ALARM_AUDIO   = 7

CONF_I2C_BUS = 'i2c_bus'

REQUIREMENTS = ['i2csense==0.0.4',
                'smbus-cffi==0.5.1']

_LOGGER = logging.getLogger(__name__)

BOARD = None

DOMAIN = 'I2C'

CONFIG_SCHEMA = vol.Schema({
    DOMAIN: vol.Schema({
        vol.Optional(CONF_I2C_BUS, default=0): cv.positive_int,
    }),
}, extra=vol.ALLOW_EXTRA)


def setup(hass, config):
    """Set up the I2C component."""

    busNum = config[DOMAIN][CONF_I2C_BUS]

    global BOARD
    try:
        BOARD = I2CBoard(busNum)
    except:
        _LOGGER.error("I2C bus %i unknown error", busNum)
        return False
  
    # TODO: should test my bus and arduino sketch here

    def stop_i2c(event):
        """Stop the I2C service."""
        BOARD.disconnect()

    def start_i2c(event):
        """Start the Arduino service."""
        hass.bus.listen_once(EVENT_HOMEASSISTANT_STOP, stop_i2c)

    hass.bus.listen_once(EVENT_HOMEASSISTANT_START, start_i2c)

    return True


class I2CBoard(object):
    """Representation of an Arduino board."""

    def __init__(self, busNum):
        """Initialize the board."""
        self._bus = smbus2.SMBus(busNum)
        self._isBusy = False
    def get_float(self, data, index, offset = 0):
        bts = data[offset+(4*index):offset+((index+1)*4)]
        res = struct.unpack("f", bytes(bts))[0]
        return round(res, 2)
  
    def get_digital(self, address, pin):
        """ get the value of a digital pin """
        # send the command that we want to execute   
        return self.perform_cmd(address, CMD_DIGITAL_READ, pin, 0)
        
    def get_readdht(self, address, pin):
        # send the command that we want to execute   
        return self.perform_cmd(address, CMD_READ_DHT, pin, 0)
        
    def get_analogue(self, address, pin):
        """ get the value of aan analogue pin """
        # send the command that we want to execute   
        return self.perform_cmd(address, CMD_ANALOG_READ, pin, 0)
        
    def perform_cmd(self, address, cmd, pin, newPinValue):
        results = None
        
        # board is busy, try again later
        if (self._isBusy):
            _LOGGER.warning("board is busy, postponing command %i on pin %i", cmd, pin)
            # give some time to finish the execution of the current command
            time.sleep(0.50)
            return self.perform_cmd(address, cmd, pin, newPinValue)
        
        # lock board as busy
        self._isBusy = True
        i2cFail = False
        data = None
        
        # send the command that we want to execute   
        try:
           self._bus.write_i2c_block_data(address, cmd, [pin, newPinValue])
        except BaseException as e:
           i2cFail = True
           _LOGGER.error("Exception occurred when writing i2c command %i to pin %i: %s", cmd, pin, str(e))
           
        # do we need a response
        # finished for now... should request an ok from the board in the future
        # release board
        needsResponse = (cmd != CMD_DIGITAL_WRITE and cmd != CMD_ALARM_AUDIO)

        # request response if we need it and writing was not a faliure
        if (i2cFail == False and needsResponse):
           # give some time to the board to execute the code and calculate results
           time.sleep(0.10)
        
           #request pre data (head and result of cmd)
           try:
            data = self._bus.read_i2c_block_data(address, 0, 16)
           except BaseException as e:
            i2cFail = True
            _LOGGER.error("Exception occurred when reading response to command %i on pin %i: %s", cmd, pin, str(e))
        
        # release board
        self._isBusy = False
        
        # done if we don't need a response from the board or we had a writing error
        if (i2cFail or needsResponse == False):
            return
        
        #parse the results
        if (data is None):
            _LOGGER.error("No data received while reading the response to command %i on pin %i", cmd, pin)
            return
        
        if (data[0] is not cmd):
            _LOGGER.error("Command miscommunication occurred when requesting command %i on pin %i", cmd, pin)
            return
            
        if (data[1] == 0):
            _LOGGER.error("Board error when requesting command %i on pin %i", cmd, pin)
            return
            
        # data was good
        results = [float('nan'), float('nan')]

        # proceed to read data
        offset = 2
        for r in range(len(results)):
            results[r] = self.get_float(data, r, offset)

        return results

    def set_digital_high(self, address, pin):
        """Set a given digital pin to high."""
        self.perform_cmd(address, CMD_DIGITAL_WRITE, pin, 1)

    def set_digital_low(self, address, pin):
        """Set a given digital pin to low."""
        self.perform_cmd(address, CMD_DIGITAL_WRITE, pin, 0)

    def disconnect(self):
        """Disconnect the board and close the serial connection."""
        self._bus.close()