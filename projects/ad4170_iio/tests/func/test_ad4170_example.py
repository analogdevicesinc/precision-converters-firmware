import decimal
import pytest
import serial
from time import sleep
import csv
import os
#import adi.ad4170      # TODO - Enable once ad4170 iio linux drivers are publicly released
from ad4170 import *    # TODO - Delete later

iio_device = { 'DEV_AD4170': 'ad4170' }
MAX_EXPECTED_VOLTAGE = 1.9
MIN_EXPECTED_VOLTAGE = 1.7

def test_ad4170(serial_port, device_name, serial_com_type, target_reset):
    uri_str = "serial:" + serial_port + ",230400"

    # Allow VCOM connection to establish upon power-up
    sleep(3)    

    # Create a new instance of AD4170 class which creates
    # a device context as well for IIO interface
    ad4170_dev = ad4170(uri_str, iio_device[device_name])

    # *********** Perform data capture check ***********
    # TODO - The first value of the captured data seems to be offset from the expected result.
    #        Therefore it is discarded from the list. Once issue is resolved in the
    #        firmware, update this file.

    # *Note: Test is done based on the assumption that, fixed DC voltage of 1.8v is applied to 
    #        chn0 of AD4170 device
    print("\nData capture test for channel 0 => \n")
    chn = 0
    ad4170_dev._rx_data_type = np.int32
    ad4170_dev._rx_stack_interleaved = True
    ad4170_dev.rx_output_type = "SI"
    ad4170_dev.rx_enabled_channels = [chn]
    ad4170_dev.rx_buffer_size = 100
    data = ad4170_dev.rx()

    # Write data into CSV file (tests/output directory)
    current_dir = os.getcwd()
    output_dir = os.path.join(current_dir, "output")
    isdir = os.path.isdir(output_dir)
    if not isdir:
        # Create 'output' directory if doesn't exists
        os.mkdir(output_dir)

    file_name = device_name + "_" + serial_com_type + "_RESULT" + ".csv"
    output_file = os.path.join(output_dir, file_name)

    with open(output_file, 'w') as result_file:
        # create the csv writer
        writer = csv.writer(result_file)

        # Write data into first row
        writer.writerow(data[1:])
    
    # Validate the data
    max_val = max(data[1:]) / 1000
    min_val = min(data[1:]) / 1000
    print("Max Voltage: {0}".format(max_val))
    print("Min Voltage: {0}".format(min_val))
    if (max_val > MAX_EXPECTED_VOLTAGE or min_val < MIN_EXPECTED_VOLTAGE):
        assert (0), "Error reading voltage!!"
