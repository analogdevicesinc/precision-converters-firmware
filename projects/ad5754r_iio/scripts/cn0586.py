# Copyright (C) 2020-2024 Analog Devices, Inc.
#
# SPDX short identifier: ADIBSD

from decimal import Decimal

from adi.attribute import attribute
from adi.context_manager import context_manager
from adi.rx_tx import tx
from adi.ad5754r import ad5754r

class cn0586(ad5754r, tx, context_manager):
    _complex_data = False
    _device_name = "ad5754r"

    def __init__(self, uri="", device_name=""):
        """ Constructor for CN0586 driver class """
        
        context_manager.__init__(self, uri, self._device_name)
        
        self._ctrl = None

        # Select the device matching device_name as working device
        for device in self._ctx.devices:
            if device.name == self._device_name:
                self._ctrl = device
                self._txdac = device
                break

        if not self._ctrl:
            raise Exception("Error in selecting AD5754R IIO Device")

        if not self._txdac:
            raise Exception("Error in selecting AD5754R IIO Device")

        self._cftl = self._ctx.find_device("cn0586");

        if not self._cftl:
            raise Exception("Error in selecting CN0586 IIO Device")

        self.output_bits = []
        for ch in self._ctrl.channels:
            name = ch.id
            self.output_bits.append(ch.data_format.bits)
            self._tx_channel_names.append(name)
            self.channel.append(self._channel(self._ctrl, name))

        tx.__init__(self)

    @property
    def hvout_state(self):
        """Get enable/disable state of the High Voltage Output"""
        return self._get_iio_dev_attr_str("hvout_state", self._cftl)

    @property
    def hvout_state_available(self):
        """Get list of all hvout enable/disable settings"""
        return self._get_iio_dev_attr_str("hvout_state_available", self._cftl)

    @hvout_state.setter
    def hvout_state(self, value):
        """Set enable/disable state of the High Voltage Output"""
        if value in self.hvout_state_available:
            self._set_iio_dev_attr_str("hvout_state", value, self._cftl)
        else:
            raise ValueError(
                "Error: hvout state not supported \nUse one of: "
                + str(self.hvout_state_available)
            )

    @property
    def hvout_range(self):
        """Get range of the High Voltage Output"""
        return self._get_iio_dev_attr_str("hvout_range", self._cftl)

    @property
    def hvout_range_available(self):
        """Get list of all hvout range settings"""
        return self._get_iio_dev_attr_str("hvout_range_available", self._cftl)

    @hvout_range.setter
    def hvout_range(self, value):
        """Set range of the High Voltage Output"""
        if value in self.hvout_range_available:
            self._set_iio_dev_attr_str("hvout_range", value, self._cftl)
        else:
            raise ValueError(
                "Error: hvout range not supported \nUse one of: "
                + str(self.hvout_range_available)
            )

    @property
    def hvout_volts(self):
        """Get volts at the High Voltage Output"""
        return self._get_iio_dev_attr_str("hvout_volts", self._cftl)

    @hvout_volts.setter
    def hvout_volts(self, value):
        """Set volts to appear at HVOUT when it's enabled"""
        self._set_iio_dev_attr_str("hvout_volts", str(value), self._cftl)
