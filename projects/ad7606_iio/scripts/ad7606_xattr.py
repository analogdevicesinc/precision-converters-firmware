# Copyright (C) 2021-22 Analog Devices, Inc.
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#     - Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     - Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in
#       the documentation and/or other materials provided with the
#       distribution.
#     - Neither the name of Analog Devices, Inc. nor the names of its
#       contributors may be used to endorse or promote products derived
#       from this software without specific prior written permission.
#     - The use of this software may or may not infringe the patent rights
#       of one or more patent holders.  This license does not release you
#       from the requirement that you obtain separate licenses from these
#       patent holders to use this software.
#     - Use of the software either in source or binary form, must be run
#       on or directly connected to an Analog Devices Inc. component.
#
# THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
# INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
# PARTICULAR PURPOSE ARE DISCLAIMED.
#
# IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, INTELLECTUAL PROPERTY
# RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
# BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
# THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#import adi.ad7606              # TODO - Enable later
#from adi.ad7606 import ad7606  # TODO - Enable later
from ad7606 import *            # TODO - Delete later
from adi.attribute import attribute
from decimal import Decimal


# Create a child class of ad7606 parent class for defining extended iio attributes (the ones which
# are not part of original linux iio drivers and created for non-linux iio applications)
class ad7606_xattr(ad7606):

    # List of extended iio channels
    xchannel = []

    def __init__(self, uri, device_name):
        super().__init__(uri, device_name)

        # Create an instances of the extended channel class
        for ch in self._ctrl.channels:
            name = ch._id
            self._rx_channel_names.append(name)
            self.xchannel.append(self._xchannel(self._ctrl, name))

        # Add the _xchannel class methods as _channel methods, so that they can be accessed with 
        # attribute name as 'channel' instead 'xchannel'
        # e.g. channel attribute 'aldo' can be accessed as either "obj.channel[chn_num].aldo" Or
        # "obj.xchannel[chn_num].aldo", where obj refers to object of 'ad7606_xattr' class
        self._channel.aldo = self._xchannel.aldo
        self._channel.dldo = self._xchannel.dldo
        self._channel.vref = self._xchannel.vref
        self._channel.vdrive = self._xchannel.vdrive
        self._channel.chn_range = self._xchannel.chn_range
        self._channel.temperature = self._xchannel.temperature
        self._channel.open_circuit_detect_manual = self._xchannel.open_circuit_detect_manual
        self._channel.open_circuit_detect_auto = self._xchannel.open_circuit_detect_auto
        self._channel.calibrate_adc_offset = self._xchannel.calibrate_adc_offset
        self._channel.calibrate_adc_gain = self._xchannel.calibrate_adc_gain

    #------------------------------------------------
    # Device extended attributes
    #------------------------------------------------
    
    @property
    def sample_rate(self):
        """AD7606 device sample_rate"""
        return int(self._get_iio_dev_attr_str("sampling_frequency"))

    @property
    def operating_mode(self):
        """AD7606 operating_mode"""
        return self._get_iio_dev_attr_str("operating_mode")

    @operating_mode.setter
    def operating_mode(self, value):
        self._set_iio_dev_attr_str("operating_mode", value)

    @property
    def power_down_mode(self):
        """AD7606 power_down_mode"""
        return self._get_iio_dev_attr_str("power_down_mode")

    @power_down_mode.setter
    def power_down_mode(self, value):
        self._set_iio_dev_attr_str("power_down_mode", value)

    #------------------------------------------------
    # Channel extended attributes
    #------------------------------------------------

    class _xchannel(attribute):

        def __init__(self, ctrl, channel_name):
            self.name = channel_name
            self._ctrl = ctrl

        @property
        def chn_range(self):
            """AD7606 chn_range"""
            return self._get_iio_attr_str(self.name, "chn_range", False)

        @chn_range.setter
        def chn_range(self, value):
            self._set_iio_attr(self.name, "chn_range", False, value)
    
        @property
        def temperature(self):
            """AD7606 temperature"""
            return float(self._get_iio_attr_str(self.name, "temperature", False))

        @property
        def vref(self):
            """AD7606 vref"""
            return float(self._get_iio_attr_str(self.name, "vref", False))
        
        @property
        def vdrive(self):
            """AD7606 vref"""
            return float(self._get_iio_attr_str(self.name, "vdrive", False))

        @property
        def aldo(self):
            """AD7606 ALDO"""
            return float(self._get_iio_attr_str(self.name, "ALDO", False))

        @property
        def dldo(self):
            """AD7606 DLDO"""
            return float(self._get_iio_attr_str(self.name, "DLDO", False))
        
        @property
        def open_circuit_detect_manual(self):
            """AD7606 open_circuit_detect_manual"""
            return self._get_iio_attr_str(self.name, "open_circuit_detect_manual", False)

        @property
        def open_circuit_detect_auto(self):
            """AD7606 open_circuit_detect_auto"""
            return self._get_iio_attr_str(self.name, "open_circuit_detect_auto", False)

        @open_circuit_detect_auto.setter
        def open_circuit_detect_auto(self, value):
            self._set_iio_attr(self.name, "open_circuit_detect_auto", False, value)
        
        @property
        def calibrate_adc_offset(self):
            """AD7606 calibrate_adc_offset"""
            return self._get_iio_attr_str(self.name, "calibrate_adc_offset", False)
        
        @property
        def calibrate_adc_gain(self):
            """AD7606 calibrate_adc_gain"""
            return self._get_iio_attr_str(self.name, "calibrate_adc_gain", False)
        
        @calibrate_adc_gain.setter
        def calibrate_adc_gain(self, value):
            self._set_iio_attr(self.name, "calibrate_adc_gain", False, value)
    