import numpy as np
from numpy import ndarray
from serial import Serial
from time import sleep
import os
import csv
from adi import ad719x

# Global variables
iio_device = { 'DEV_AD7193': 'ad7193'}
MAX_EXPECTED_VOLTAGE = 3.4
MIN_EXPECTED_VOLTAGE = 3.2

def test_ad719x(serial_port, device_name, serial_com_type, target_reset):
    uri_str = "serial:" + serial_port + ",230400"

    # Allow VCOM connection to establish upon power-up
    sleep(3)    

    # Create a new instance of AD7193 class which creates
    # a device context as well for IIO interface
    ad7193_dev = ad719x(uri_str, iio_device[device_name])

    # *********** Perform data capture check ***********

    # Note: Test is done based on the assumption that, a fixed DC voltage of 3.3v is applied to 
    #       chn2 (AIN3) of AD7193 device configured in unipolar and pseudo-differential mode.
    print("\nData capture test for channel 2 => \n")
    chn = 2
    ad7193_dev._rx_data_type = np.uint32
    ad7193_dev.rx_enabled_channels = [chn]
    ad7193_dev.rx_buffer_size = 100
    raw_data = ad7193_dev.rx()

    data = ndarray((ad7193_dev.rx_buffer_size,),int)
    scale = ad7193_dev.channel[chn].scale
    offset = ad7193_dev.channel[chn].offset
    for indx in range(ad7193_dev.rx_buffer_size):
        data[indx] = (raw_data[indx] + offset) * scale

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