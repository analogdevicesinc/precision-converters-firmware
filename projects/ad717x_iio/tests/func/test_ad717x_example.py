import decimal
import pytest
import serial
import csv
from time import sleep
import os
from adi.ad717x import *

iio_device = { 'DEV_AD4111': 'ad4111' }
MAX_EXPECTED_VOLTAGE = 2.0
MIN_EXPECTED_VOLTAGE = 1.8

def test_ad717x(serial_port, device_name, serial_com_type, target_reset):
    uri_str = "serial:" + serial_port + ",230400"

    # Allow VCOM connection to establish upon power-up
    sleep(3)    

    # Create a new instance of AD17x class which creates
    # a device context as well for IIO interface
    ad717x_dev = ad717x(uri_str, iio_device[device_name])

    # *********** Perform data capture check ***********

    # *Note: Test is done based on the assumption that, fixed DC voltage of 1.8v is applied to 
    #        chn0 of AD717x device
    print("\nData capture test for channel 0 => \n")
    # The 'temperature' channel is considered as index 0 and voltage channels 0-7 as index 1-7 respectively
    chn = 1
    ad717x_dev._rx_data_type = np.int32
    ad717x_dev.rx_output_type = "raw"
    ad717x_dev.rx_enabled_channels = [chn]
    ad717x_dev.rx_buffer_size = 100
    raw_data = ad717x_dev.rx()
    data = ad717x_dev.to_volts(chn, raw_data)

    # Apply the scale factor to the offset value and update the actual voltage value
    for sampleIndex in range(0, len(data)):
        data[sampleIndex] = (data[sampleIndex]+(ad717x_dev.channel[chn].offset)*(ad717x_dev.channel[chn].scale))

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
