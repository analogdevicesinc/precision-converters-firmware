import decimal
import pytest
import serial
from time import sleep
import csv
import os
from numpy import ndarray
from adi.ad7134 import *    # TODO - Delete later

iio_device = { 'DEV_AD7134': 'ad7134' }
MAX_EXPECTED_VOLTAGE = 1.9
MIN_EXPECTED_VOLTAGE = 1.7

def test_ad7134(serial_port, device_name, target_reset):
    uri_str = "serial:" + serial_port + ",230400"

    # Allow VCOM connection to establish upon power-up
    sleep(3)    

    # Create a new instance of AD7134 class which creates
    # a device context as well for IIO interface
    ad7134_dev = ad7134(uri_str, iio_device[device_name])

    # *********** Perform data capture check ***********
    # *Note: Test is done based on the assumption that, fixed DC voltage of 1.8v is applied to 
    #        chn0 of AD7134 device
    print("\nData capture test for channel 0 => \n")
    chn = 0
    ad7134_dev._rx_data_type = np.int32
    ad7134_dev.rx_output_type = "raw"
    ad7134_dev._rx_stack_interleaved = True
    # Enabling all channels as the FW doesn't suport enabling single channel
    ad7134_dev.rx_enabled_channels = ["voltage0","voltage1","voltage2","voltage3"]
    ad7134_dev.rx_buffer_size = 100

    # Read the Raw Attribute once to update the scale and offset values
    adc_raw = ad7134_dev.channel[chn].raw
    offset =  ad7134_dev.channel[chn].offset
    scale = ad7134_dev.channel[chn].scale
    
    chn_data = ad7134_dev.rx()

    data = ndarray((ad7134_dev.rx_buffer_size,),int)
    
    # Extract the data of the enabled channel set in 'chn'
    for sample_index in range(len(data)):
        # Apply the Offset and Scale to the raw data of enabled channel 'chn'
        data[sample_index] = (chn_data[chn][sample_index]+ offset)*scale
    
    
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
