import decimal
import pytest
import serial
from time import sleep
import csv
import os
from numpy import ndarray
from adi import ad405x #TODO: Uncomment this later when driver is released to pyadi-iio
from ad405x import * #TODO: Remove this later when driver is released to pyadi-iio


iio_device = { 'DEV_AD4052': 'ad4052' }
MAX_EXPECTED_VOLTAGE = 1.9
MIN_EXPECTED_VOLTAGE = 1.7

def test_ad405x(serial_port, device_name, target_reset):
    uri_str = "serial:" + serial_port + ",230400"

    # Allow VCOM connection to establish upon power-up
    sleep(3)    

    # Create a new instance of ad405x class which creates
    # a device context as well for IIO interface
    ad405x_dev = ad405x(uri_str, iio_device[device_name])

    # *********** Perform data capture check ***********
    # *Note: Test is done based on the assumption that, fixed DC voltage of 1.8v is applied to 
    #        chn0 of ad405x device
    print("\nData capture test for channel 0 => \n")
    ad405x_dev._rx_data_type = np.int32
    ad405x_dev.rx_output_type = "SI"
    ad405x_dev._rx_stack_interleaved = True
    ad405x_dev.rx_enabled_channels = [0]
    ad405x_dev.rx_buffer_size = 100
    
    # Note: The chn_data holds the sampled values of all channels irrespective of the chosen
    # channel in rx_enabled_channels.
    chn_data = ad405x_dev.rx()

    data = ndarray((ad405x_dev.rx_buffer_size,),int)
    
    # Extract the data of enabled channels in rx_enabled_channels
    for sample_index in range(0, len(data)):
        data[sample_index] = chn_data[ad405x_dev.rx_enabled_channels[0]]
        sample_index = sample_index + len(ad405x_dev.channel)
    
    
    # Write data into CSV file (tests/output directory)
    current_dir = os.getcwd()
    output_dir = os.path.join(current_dir, "output")
    isdir = os.path.isdir(output_dir)
    if not isdir:
        # Create 'output' directory if doesn't exists
        os.mkdir(output_dir)

    file_name = device_name + "_" + "RESULT" + ".csv"
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