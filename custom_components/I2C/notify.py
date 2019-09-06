"""
Support for HTU21D temperature and humidity sensor.

For more details about this platform, please refer to the documentation at
https://home-assistant.io/components/sensor.htu21d/
"""
import logging

import voluptuous as vol

import homeassistant.helpers.config_validation as cv
from homeassistant.components.notify import (
    ATTR_TITLE, ATTR_TITLE_DEFAULT, PLATFORM_SCHEMA, BaseNotificationService)
from homeassistant.const import CONF_PASSWORD
import custom_components.I2C as I2C

DEPENDENCIES = ['I2C']

_LOGGER = logging.getLogger(__name__)

CMD_ALARM_AUDIO   = 7
ADDRESS = 4
CONF_I2C_ADDRESS = 'i2c_address'
CONF_PIN = 'pin'
DEFAULT_I2C_ADDRESS = '0x04'

PLATFORM_SCHEMA = PLATFORM_SCHEMA.extend({
    vol.Required(CONF_PIN): cv.positive_int,
    vol.Optional(CONF_I2C_ADDRESS, default=DEFAULT_I2C_ADDRESS): cv.string,
})

def get_service(hass, config, discovery_info=None):
    # Verify that I2C board is present
    if I2C.BOARD is None:
        _LOGGER.error("A connection has not been made to the I2C board")
        return False

    """Get the Simplepush notification service."""
    return I2CAlarmNotificationService(config)


class I2CAlarmNotificationService(BaseNotificationService):
    """Implementation of the notification service for Simplepush."""

    def __init__(self, config):
        """Initialize the Simplepush notification service."""
        self._pin = config.get(CONF_PIN)
        # set up address
        ADDRESS = int(config.get(CONF_I2C_ADDRESS),16)
        self._address = ADDRESS

        self.msg_handler = I2C.BOARD.perform_cmd

    def send_message(self, message=None, **kwargs):
        parSend = int(message)
        self.msg_handler(self._address, CMD_ALARM_AUDIO, self._pin, parSend);