import sys
sys.path.insert(0,'../../scripts')
import pytest
import csv
from time import sleep
import os
import numpy as np
from ad7091r_xattr import *

# Global variables
iio_device = {'AD7091R_8': 'ad7091r-8'}
MAX_EXPECTED_VOLTAGE = 1.9
MIN_EXPECTED_VOLTAGE = 1.7

def test_ad7091r(serial_port, device_name, serial_com_type, target_reset):
    uri_str = "serial:" + serial_port + ",230400"

    # Allow VCOM connection to establish upon power-up
    sleep(3)

    # Create a new instance of AD579x class which creates
    # a device context as well for IIO interface
    dev = ad7091rx_xattr(uri_str, iio_device[device_name])

    # *********** Perform data capture check ***********

    # *Note: Test is done based on the assumption that, default DC voltage of
    # 1.8v is applied to chn0 of AD4696 device
    print("\nData capture test for channel 0 => \n")
    chn = 0
    dev._rx_data_type = np.uint16
    dev.rx_output_type = "SI"
    dev.rx_enabled_channels = [chn]
    dev.rx_buffer_size = 100
    dev._rx_stack_interleaved = True

    data = dev.rx()
    #dev._ctx.__del__()

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
        writer.writerow(data)

    # Validate the data
    max_voltage = max(data) / 1000
    min_voltage = min(data) / 1000
    print("Max Voltage: {0}".format(max_voltage))
    print("Min Voltage: {0}".format(min_voltage))
    assert (max_voltage < MAX_EXPECTED_VOLTAGE and min_voltage > MIN_EXPECTED_VOLTAGE), "Error reading voltage!!"

