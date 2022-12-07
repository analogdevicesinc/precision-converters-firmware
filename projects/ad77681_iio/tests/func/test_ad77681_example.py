import decimal
import pytest
import serial
import csv
from time import sleep
import os
#import adi.ad77681      # TODO - Enable later
from ad77681 import *    # TODO - Delete later

iio_device = {
                'DEV_AD77681': 'ad7768-1'
             }
'''
Expected voltage is capture base on the computation below:
Voltage = Input Voltage * Constant Voltage (2.67V)
Constant Voltage was derived from the circuit gain.
e.g.
V = 0.9 * 2.67 = 2.403v
'''
MAX_EXPECTED_VOLTAGE = 2.5
MIN_EXPECTED_VOLTAGE = 2.3

def test_ad77681(serial_port, device_name, serial_com_type, target_reset):
    uri_str = "serial:" + serial_port + ",230400"

    # Allow VCOM connection to establish upon power-up
    sleep(3)    

    # Create a new instance of AD77681 class which creates
    # a device context as well for IIO interface
    ad77681_dev = ad77681(uri_str, iio_device[device_name])

    # *********** Perform data capture check ***********

    # *Note: Test is done based on the assumption that, fixed DC voltage of 1.8v is applied to 
    #        chn0 of AD77681 device
    print("\nData capture test for channel 0 => \n")
    # The AD77681 has only 1 channel respectively
    chn = 0
    ad77681_dev._rx_data_type = np.int32
    ad77681_dev.rx_output_type = "SI"
    ad77681_dev.rx_enabled_channels = [chn]
    ad77681_dev.rx_buffer_size = 100
    ad77681_dev._rx_stack_interleaved = True
    data = ad77681_dev.rx()

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
