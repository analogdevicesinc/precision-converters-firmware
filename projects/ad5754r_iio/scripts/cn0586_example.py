#################

# Copyright (C) 2024 Analog Devices, Inc.
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

import numpy as np

from cn0586 import *        # TODO - Delete later
from adi.cn0586 import *    # TODO - Enable later

# Set up CN0586
cn0586_dev = cn0586(uri="serial:COM9,230400,8n1n")

# available ranges: 0V_to_100V M100V_to_100V M50V_to_50V 0V_to_200V
cn0586_dev.hvout_range = "M100V_to_100V"
cn0586_dev.hvout_volts = 0
cn0586_dev.hvout_state = "Enabled"

# Get HVOUT state, range and volts
print ("Initial Configuration - ")
print(f"CN0586 HVOUT State: {cn0586_dev.hvout_state}")
print(f"CN0586 HVOUT Range: {cn0586_dev.hvout_range}")
print(f"CN0586 HVOUT Volts: {cn0586_dev.hvout_volts}")
print()



#----- Test Wavegen (Only Channel 0 allowed) -----#
chn = 0
# print(f"Sampling/Update Rate: {cn0586_dev.sampling_frequency}")
cn0586_dev._tx_data_type = np.uint16
cn0586_dev.tx_cyclic_buffer = True
cn0586_dev.tx_enabled_channels = [chn]

# sampling_frequency is the output data rate of the DAC used for data streaming
cn0586_dev.sampling_frequency = 50000 #default frequency for sine wave example

# two data points for square wave
#arr = [0x4000, 0xC000]

# 50 data points for sinewave
arr = [32768,36875,40917,44831,48554,52029,55199,58016,60435,62417,63932,64956,65471,65471,64956,63932,62417,60435,58016,55199,52029,48554,44831,40917,36875,32768,28661,24619,20705,16982,13507,10337,7520,5101,3119,1604,580,65,65,580,1604,3119,5101,7520,10337,13507,16982,20705,24619,28661]

print()
print(f"Streaming {len(arr)} samples at {cn0586_dev.sampling_frequency}Hz")
print(f"Output frequency: {int(cn0586_dev.sampling_frequency)/len(arr)} Hz")
print()

data = np.array(arr)
cn0586_dev.tx(data)

# In cyclic mode, samples in the supplied buffer are pushed repeatedly to the DAC
# To observe the output for x seconds, replace 1 with x in the sleep function argument
import time
time.sleep(10)

cn0586_dev.tx_destroy_buffer()
#cn0586_dev.hvout_volts = -20
cn0586_dev.hvout_state = "Disabled"

print()
print ("Final State - ")
print(f"CN0586 HVOUT State: {cn0586_dev.hvout_state}")
print(f"CN0586 HVOUT Range: {cn0586_dev.hvout_range}")
print(f"CN0586 HVOUT Volts: {cn0586_dev.hvout_volts}")