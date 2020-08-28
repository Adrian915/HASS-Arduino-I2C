"""
Support for HTU21D temperature and humidity sensor.

For more details about this platform, please refer to the documentation at
https://home-assistant.io/components/sensor.htu21d/
"""
import logging
import math

import voluptuous as vol
from datetime import timedelta

from homeassistant.components.sensor import PLATFORM_SCHEMA
from homeassistant.helpers.entity import Entity
from homeassistant.util import Throttle
import homeassistant.helpers.config_validation as cv
from homeassistant.const import (
    ATTR_DEVICE_CLASS,
    DEVICE_CLASS_HUMIDITY,
    DEVICE_CLASS_TEMPERATURE,
    TEMP_CELSIUS,
    UNIT_PERCENTAGE,
    TIME_HOURS,
    UNIT_PERCENTAGE, 
    CONF_NAME, 
    CONF_MONITORED_CONDITIONS)
import custom_components.I2C as I2C

REQUIREMENTS = ['i2csense==0.0.4',
                'smbus-cffi==0.5.1']

_LOGGER = logging.getLogger(__name__)

ADDRESS = 4

MAX_UPDATE_FALIURES = 4

CONF_PINS = 'pins'

CONF_I2C_ADDRESS = 'i2c_address'
DEFAULT_I2C_ADDRESS = '0x04'
CONF_UNIQUE_ID = 'unique_id'

SENSOR_TEMPERATURE = 'temperature'
SENSOR_HUMIDITY = 'humidity'
SENSOR_ANALOGUE_VALUE = 'analogue_value'

SENSOR_TYPES = {
    SENSOR_TEMPERATURE: ['Temperature', TEMP_CELSIUS, "mdi:thermometer"],
    SENSOR_HUMIDITY: ["Humidity", UNIT_PERCENTAGE, "mdi:water-percent"],
    SENSOR_ANALOGUE_VALUE: ['Percentage', UNIT_PERCENTAGE, "mdi:theme-light-dark"],
}
DEFAULT_NAME = 'I2C Sensor'

# DHT11 is able to deliver data once per second, DHT22 once every two
MIN_TIME_BETWEEN_UPDATES = timedelta(seconds=3)

PIN_SCHEMA = vol.Schema({
    vol.Required(CONF_NAME): cv.string,
    vol.Optional(CONF_MONITORED_CONDITIONS, default=[]):
       vol.All(cv.ensure_list, [vol.In(SENSOR_TYPES)]),
    vol.Optional(CONF_UNIQUE_ID): cv.string,
})

PLATFORM_SCHEMA = PLATFORM_SCHEMA.extend({
    vol.Optional(CONF_I2C_ADDRESS, default=DEFAULT_I2C_ADDRESS): cv.string,
    vol.Required(CONF_PINS, default={}):
       vol.Schema({cv.positive_int: PIN_SCHEMA}),
})


def setup_platform(hass, config, add_devices, discovery_info=None):
    """Set up the sensor."""
    
    # Verify that I2C board is present
    if I2C.BOARD is None:
        _LOGGER.error("A connection has not been made to the I2C board")
        return False

    # set up address
    ADDRESS = int(config.get(CONF_I2C_ADDRESS),16)

    #set up my devices
    pins = config.get(CONF_PINS)

    devices = []
    for pinnum, device_conf in pins.items():
        monitored_conditions = device_conf.get(CONF_MONITORED_CONDITIONS)
        try:
            for variable in monitored_conditions:
                devices.append(I2CDHTSensor(pinnum, ADDRESS, I2C.BOARD, variable, device_conf))
        except KeyError:
            continue
  
    add_devices(devices)


class I2CDHTSensor(Entity):
    """Representation of a I2C sensor."""

    def __init__(self, pin, address, board, sensor_type, options):
        """Initialize the Pin."""
        self._pin = pin
        self._type = sensor_type
        self.client_name = options.get(CONF_NAME)
        self._name = "%s (%s)" % (self.client_name, SENSOR_TYPES[sensor_type][0])
        self._address = address
        self._board = board
        self._unit_of_measurement = SENSOR_TYPES[sensor_type][1]
        self._value = None
        self._fail_attempts = 0
        self._unique_id = options.get(CONF_UNIQUE_ID)

    @property
    def name(self):
        """Get the name of the pin."""
        return self._name
        
    @property
    def icon(self):
        """Icon to use in the frontend."""
        return SENSOR_TYPES[self._type][2]

    @property
    def state(self):
        """Return the state of the sensor."""
        return self._value
        
    @property
    def unit_of_measurement(self):
        """Return the unit of measurement of this entity, if any."""
        return self._unit_of_measurement

    @Throttle(MIN_TIME_BETWEEN_UPDATES)
    def update(self):
        """Get the latest value from the pin."""
        if (self._board._isBusy):
            _LOGGER.warning("board is busy, skipping update of device " + self._name)
        else: 
            result = None
                
            # set the results
            if (self._type == SENSOR_ANALOGUE_VALUE):
                result = self._board.get_analogue(self._address, self._pin)
            else:
                result = self._board.get_readdht(self._address, self._pin)
                
            if (result is None):
                if (self._fail_attempts < MAX_UPDATE_FALIURES):
                    self._fail_attempts += 1
                else:
                    self._value = float('nan')
            else:
                self._fail_attempts = 0
                if (self._type == SENSOR_ANALOGUE_VALUE):
                    # raw is between 0(high brightness) and 1023 (darkest)
                    pLight = (result[0]/1023) * 100
                    self._value = int(pLight)
                elif (self._type == SENSOR_TEMPERATURE):
                    self._value = result[0]
                elif (self._type == SENSOR_HUMIDITY):
                    self._value = result[1]