# Copyright (C) 2024, 2026 Analog Devices, Inc.
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

from time import sleep
from pynput import keyboard
import os
import csv
import math
#from adi import ad4692 # TODO - Add later
from ad4692 import *    # TODO - Delete later

# Global variables
line = 0
writer = 0
run_continuous = False
iterations = 0
data_capture_abort = False
chn_count = 0
data_list = []

def key_press_event(key):
    global data_capture_abort
    data_capture_abort = True

def init_data_capture():
    global device
    global data_list
    global chn_count
    global listener
    global samples_block

    ######## User configuration ##########
    # Configure the backend for PC to IIOD interface
    uri = "serial:COM29,230400"  # For UART, baud rate must be same as set in the FW. COM port is physical Or VCOM.
    device_name = "ad4692"      # Name of the device must be same as set in the FW.
    ######################################

    # Create an IIO device context
    device = ad4692(uri, device_name)
    device._ctx.set_timeout(100000)

    ######## User configuration ##########
    # Channels to be captured e.g. ["voltage0"]: 1chn, ["voltage0, voltage1"]: 2chns
    device.rx_enabled_channels = ["voltage0", "voltage1", "voltage2", "voltage3", "voltage4", "voltage5", "voltage6", "voltage7",
                                "voltage8", "voltage9", "voltage10", "voltage11", "voltage12", "voltage13", "voltage14", "voltage15"]
    # The block of samples to be captured. Total samples are received in multiple iterations or blocks
    samples_block = 400    # The samples needs to be captured in smaller blocks due to limitations
                           # of buffer size (RAM) in the firmware and IIO client timeout factor.
                           # Request to capture samples more than buffer size, will be ignored by firmware.
                           # Large time taken to read the samples from device, may cause timeout on IIO client.
    ######################################

    # Get the channels count from user
    chn_count = len(device.rx_enabled_channels)

    # Store the rx buffer size and rx data type based on input channels
    device.rx_buffer_size = samples_block    # Size of the IIO buffer (buffer is submitted during call to rx() method)
    device._rx_data_type = np.uint16     # size of ADC sample
    device._rx_stack_interleaved = True

    listener = keyboard.Listener(on_press=key_press_event)
    listener.start()

def read_user_inputs():
    global iterations
    global run_continuous
    global device
    global chn_count
    global samples_block

    samples_count = int(input("Enter the number of samples to be captured \n\
                                0: Unlimited \n\
                                <50-1000000>: "))

    if (samples_count == 0):
        run_continuous = True
    else:
        run_continuous = False
        if (samples_count <= samples_block):
            device.rx_buffer_size = samples_count
            iterations = 1
        else:
            iterations = math.ceil(samples_count / samples_block)

def init_data_logger():
    global writer
    global chn_count
    file_name = "adc_data_capture.csv"
    current_dir = os.getcwd()
    output_file = os.path.join(current_dir, file_name)
    result_file = open(output_file, 'w', newline="")
    writer = csv.writer(result_file)
    row = []
    # Write the channels list header
    for chn in range(0,chn_count):
        item = "Ch {}".format(chn)
        row.insert(chn, item)
    writer.writerow(row)

def read_buffered_data():
    global line
    global writer
    global device

    # Receive data from device
    data = device.rx()
    if (line == 0):
        print("Data capture started >>")
    print("."*line, end="\r")

    if (chn_count == 1):
        # Convert 1-D array to 2-D array
        data_arr = np.reshape(data, (-1,1))
    else:
        # Transpose data from N-D data array
        data_arr = np.transpose(data)

    writer.writerows(data_arr)

    line = line+1
    if (line == 100):
        line = 1
        print("\n", end="\r")

def do_data_capture():
    global iterations
    global run_continuous
    global data_capture_abort

    done = False
    if (run_continuous == True):
        print("Press any key to stop data capture..")
        sleep(2)
        data_capture_abort = False
        while not data_capture_abort:
            read_buffered_data()
    else:
        for val in range(0,iterations):
            read_buffered_data()

    print("\r\nData capture finished\r\n")

def exit():
    global listener
    global writer
    global device

    # Delete the objects
    del listener
    del writer
    del device

def main():
    init_data_capture()
    init_data_logger()
    read_user_inputs()
    do_data_capture()
    exit()

if __name__ == "__main__":
    main()