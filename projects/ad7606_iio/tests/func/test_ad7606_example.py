import decimal
import pytest
import serial
from time import sleep
import csv
import os
#import adi.ad7606      # TODO - Enable later
from ad7606 import *    # TODO - Delete later
from ad7606_xattr import *

iio_device = { 'DEV_AD7606B': 'ad7606b',
               'DEV_AD7606C_16': 'ad7606c-16',
               'DEV_AD7606C_18': 'ad7606c-18'
             }
MAX_EXPECTED_VOLTAGE = 3.8
MIN_EXPECTED_VOLTAGE = 3.0

adc_exp_operating_mode = "2 (Auto Standby Mode)"
adc_exp_oversampling_ratio = "1 (oversampling by 2)"
adc_chn_range = ["0 (+/-2.5V SE)", "1 (+/-5.0V SE)", "2 (+/-10.0V SE)", "2 (+/-10.0V SE)", "1 (+/-5.0V SE)", "0 (+/-2.5V SE)", "0 (+/-2.5V SE)", "1 (+/-5.0V SE)"]

def test_ad7606(serial_port, device_name, serial_com_type, target_reset):
    uri_str = "serial:" + serial_port + ",230400"

    # Allow VCOM connection to establish upon power-up
    sleep(3)    

    # Create a new instance of AD7606 class which creates
    # a device context as well for IIO interface
    ad7606 = ad7606_xattr(uri_str, iio_device[device_name])

    # *********** Perform channel attributes check ***********

    for chn in range(0,8):
        print('\nChannel {0} => \n'.format(chn))

        offset_calibration = ad7606.channel[chn].calibrate_adc_offset
        if (offset_calibration == "ADC Calibration Done"):
            print("adc offset calibration done")
        else:
            assert (0), "adc offset calibration error!!"

        ad7606.channel[chn].calibrate_adc_gain = "2"
        if (ad7606.channel[chn].calibrate_adc_gain == "Calibration Done (Rfilter=2 K)"):
            print("adc gain calibration done")
        else:
            assert (0), "adc gain calibration error!!"

        # Set range to 10V SE for scanning multiplexer inputs
        ad7606.channel[chn].chn_range = "2 (+/-10.0V SE)"

        aldo = ad7606.channel[chn].aldo
        print('aldo: {0}'.format(aldo))
        assert (1.7 <= float(aldo) <= 2.0), "Error reading ALDO voltage!!"

        dldo = ad7606.channel[chn].dldo
        print('dldo: {0}'.format(dldo))
        assert (1.7 <= float(dldo) <= 2.0), "Error reading DLDO voltage!!"

        vref = ad7606.channel[chn].vref
        print('vref: {0}'.format(vref))
        assert (2.3 <= float(vref) <= 2.6), "Error reading VREF voltage!!"

        vdrive = ad7606.channel[chn].vdrive
        print('vdrive: {0}'.format(vdrive))
        assert (3.2 <= float(vdrive) <= 3.5), "Error reading Vdrive voltage!!"

        temperature = ad7606.channel[chn].temperature
        print('temperature: {0}'.format(temperature))
        assert (10.0 <= float(temperature) <= 50.0), "Error reading temperature!!"

        ad7606.channel[chn].chn_range = adc_chn_range[chn]
        print('channel range: {0}'.format(ad7606.channel[chn].chn_range))
        assert (ad7606.channel[chn].chn_range != adc_chn_range[chn]), "Error setting channel rang!!"

    # *********** Perform data capture check ***********

    # *Note: Test is done based on the assumption that, fixed DC voltage of 3.3v is applied to 
    #        chn0 of AD7606B device
    print("\nData capture test for channel 0 => \n")
    chn = 0
    ad7606.channel[chn].chn_range = "2 (+/-10.0V SE)"
    ad7606.rx_output_type = "SI"
    ad7606.rx_enabled_channels = [chn]
    ad7606.rx_buffer_size = 100
    ad7606._rx_stack_interleaved = True
    data = ad7606.rx()

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

    # *********** Perform device attributes check ***********
    
    # Set oversampling ratio
    # TODO - There is bug in the pyadi-iio ad7606 module w.r.t. this attribute. Update this file
    #        once that bug is fixed
    #ad7606.oversampling_ratio = adc_exp_oversampling_ratio
    #print("\nOversampling Ratio: {0}".format(ad7606.oversampling_ratio))
    #assert (ad7606.oversampling_ratio != adc_exp_oversampling_ratio), "Error setting ADC oversampling ratio"

    # Put into auto standby mode (reset needed to recover to normal mode)
    ad7606.operating_mode = adc_exp_operating_mode
    print("\nOperating Mode: {0}".format(ad7606.operating_mode))
    assert (ad7606.operating_mode != adc_exp_operating_mode), "Error setting ADC operating mode"
