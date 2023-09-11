import numpy as np
import decimal
import pytest
import serial
from time import sleep
import csv
import os
from numpy import ndarray
from adi.ad2s1210 import *

iio_device = { 'DEV_AD2S1210': 'ad2s1210',}
#max angle is 2pi radians
MAX_EXPECTED_ANGLE = 3.5
MIN_EXPECTED_ANGLE = 2.5

def test_ad2s1210(serial_port, device_name, serial_com_type, target_reset):
    uri_str = "serial:" + serial_port + ",230400"

    # Allow VCOM connection to establish upon power-up
    sleep(3)    

    # Create a new instance of AD2S1210 class which creates
    # a device context as well for IIO interface
    ad2s1210_dev = ad2s1210(uri_str)

    # *********** Perform data capture check ***********

    print("\nData capture test for channel 0 => \n")
    ad2s1210_dev._rx_data_type = np.uint16
    ad2s1210_dev.rx_output_type = "SI"
    ad2s1210_dev.rx_enabled_channels = ['angl0']
    ad2s1210_dev.rx_buffer_size = 100
    ad2s1210_dev._rx_stack_interleaved = True
    
    data = ad2s1210_dev.rx()

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
    max_angle = max(data)
    min_angle = min(data)
    print("Max Angle: {0}".format(max_angle))
    print("Min Angle: {0}".format(min_angle))
    assert (max_angle < MAX_EXPECTED_ANGLE and min_angle > MIN_EXPECTED_ANGLE), "Error reading angle!!"
