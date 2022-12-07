# Copyright (C) 2022 Analog Devices, Inc.
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

from adi import ad4130
from adi.attribute import attribute
from decimal import Decimal

# Create a child class of ad4130 parent class for defining extended iio attributes (the ones which
# are not part of original linux iio drivers and created for non-linux iio applications)
class ad4130_xattr(ad4130):

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
        # e.g. channel attribute 'system_calibration' can be accessed as either "obj.channel[chn_num].system_calibration" Or
        # "obj.xchannel[chn_num].system_calibration", where obj refers to object of 'ad4130_xattr' class
        self._channel.system_calibration = self._xchannel.system_calibration
        self._channel.internal_calibration = self._xchannel.internal_calibration
        self._channel.loadcell_offset_calibration = self._xchannel.loadcell_offset_calibration
        self._channel.loadcell_gain_calibration = self._xchannel.loadcell_gain_calibration

    #------------------------------------------------
    # Device extended attributes
    #------------------------------------------------
    
    @property
    def demo_config(self):
        """AD4130 demo mode config"""
        return self._get_iio_dev_attr_str("demo_config")

    @property
    def sample_rate(self):
        """AD4130 device sample_rate"""
        return int(self._get_iio_dev_attr_str("sampling_frequency"))

    #------------------------------------------------
    # Channel extended attributes
    #------------------------------------------------

    class _xchannel(attribute):

        def __init__(self, ctrl, channel_name):
            self.name = channel_name
            self._ctrl = ctrl

        @property
        def system_calibration(self):
            """AD4130 channel system calibration"""
            return self._get_iio_attr_str(self.name, "system_calibration", False)

        @system_calibration.setter
        def system_calibration(self, value):
            self._set_iio_attr(self.name, "system_calibration", False, value)

        @property
        def internal_calibration(self):
            """AD4130 channel internal calibration"""
            return self._get_iio_attr_str(self.name, "internal_calibration", False)

        @internal_calibration.setter
        def internal_calibration(self, value):
            self._set_iio_attr(self.name, "internal_calibration", False, value)

        @property
        def loadcell_offset_calibration(self):
            """AD4130 loadcell offset calibration"""
            return self._get_iio_attr_str(self.name, "loadcell_offset_calibration", False)

        @loadcell_offset_calibration.setter
        def loadcell_offset_calibration(self, value):
            self._set_iio_attr(self.name, "loadcell_offset_calibration", False, value)

        @property
        def loadcell_gain_calibration(self):
            """AD4130 loadcell gain calibration"""
            return self._get_iio_attr_str(self.name, "loadcell_gain_calibration", False)

        @loadcell_gain_calibration.setter
        def loadcell_gain_calibration(self, value):
            self._set_iio_attr(self.name, "loadcell_gain_calibration", False, value)
    