"""
Support for HTU21D temperature and humidity sensor.

For more details about this platform, please refer to the documentation at
https://home-assistant.io/components/sensor.htu21d/
"""
import logging

import voluptuous as vol

from homeassistant.components.sensor import PLATFORM_SCHEMA
from homeassistant.components.switch import (SwitchEntity, PLATFORM_SCHEMA)
import homeassistant.helpers.config_validation as cv

from homeassistant.const import (
    CONF_NAME,
    ATTR_FRIENDLY_NAME
)

import custom_components.I2C as I2C


DEPENDENCIES = ['I2C']

_LOGGER = logging.getLogger(__name__)

ADDRESS = 4

CONF_PINS = 'pins'
CONF_NEGATE = 'negate'
CONF_INITIAL = 'initial'
CONF_UNIQUE_ID = 'unique_id'

CONF_I2C_ADDRESS = 'i2c_address'
DEFAULT_I2C_ADDRESS = '0x04'

DEFAULT_NAME = 'I2C Switch'

PIN_SCHEMA = vol.Schema({
    vol.Required(CONF_NAME): cv.string,
    vol.Optional(CONF_INITIAL, default=False): cv.boolean,
    vol.Optional(CONF_NEGATE, default=False): cv.boolean,
    vol.Optional(ATTR_FRIENDLY_NAME): cv.string,
    vol.Optional(CONF_UNIQUE_ID): cv.string,
})

PLATFORM_SCHEMA = PLATFORM_SCHEMA.extend({
    vol.Optional(CONF_I2C_ADDRESS, default=DEFAULT_I2C_ADDRESS): cv.string,
    vol.Required(CONF_PINS, default={}):
       vol.Schema({cv.positive_int: PIN_SCHEMA}),
})


def setup_platform(hass, config, add_devices, discovery_info=None):
    """Set up the switch."""
 
    # Verify that I2C board is present
    if I2C.BOARD is None:
        _LOGGER.error("A connection has not been made to the I2C board")
        return False

    # set up address
    ADDRESS = int(config.get(CONF_I2C_ADDRESS),16)
 
    #set up my switches
    pins = config.get(CONF_PINS)

    switches = []
    for pinnum, pin in pins.items():
        switches.append(I2CSwitch(pinnum, ADDRESS, I2C.BOARD, pin))
    add_devices(switches)


class I2CSwitch(SwitchEntity):
    """Representation of a I2C switch."""

    def __init__(self, pin, address, board, options):
        """Initialize the Pin."""
        self._pin = pin
        self._name = options.get(CONF_NAME)
        self._address = address
        self._board = board
        self._state = options.get(CONF_INITIAL)
        self._negate = options.get(CONF_NEGATE)
        self._friendly_name = options.get(ATTR_FRIENDLY_NAME)
        self._unique_id = options.get(CONF_UNIQUE_ID)
        
        if  self._negate:
            self.turn_on_handler = I2C.BOARD.set_digital_low
            self.turn_off_handler = I2C.BOARD.set_digital_high
        else:
            self.turn_on_handler = I2C.BOARD.set_digital_high
            self.turn_off_handler = I2C.BOARD.set_digital_low

        (self.turn_on_handler if self._state else self.turn_off_handler)(address, pin)

    @property
    def name(self):
        """Get the name of the pin."""
        return self._name
        
    @property
    def unique_id(self):
        """Return the unique id of this switch."""
        return self._unique_id

    @property
    def is_on(self):
        """Return true if pin is high/on."""
        return self._state

    def turn_on(self, **kwargs):
        """Turn the pin to high/on."""
        self._state = True
        self.turn_on_handler(self._address, self._pin)

    def turn_off(self, **kwargs):
        """Turn the pin to low/off."""
        self._state = False
        self.turn_off_handler(self._address, self._pin)