import numpy as np
from serial import Serial
from time import sleep
from pynput import keyboard
import sys
import select
import os
import csv
import math
from adi import ad719x

# Global variables
line = 0
writer = 0
run_continuous = False
iterations = 0
data_capture_abort = False
chn_count = 0
data_list = []
samples_count = 0
adc_voltage_sqr_avg = 0
adc_voltage_avg = 0
sample_volt = 0

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
    uri = "serial:COM22,230400"  # For UART, baud rate must be same as set in the FW. COM port is physical Or VCOM.
    device_name = "ad7191"      # Name of the device must be same as set in the FW.
    ######################################
   
    # Create an IIO device context
    device = ad719x(uri, device_name)
    device._ctx.set_timeout(1000000)  # Should be long enough to tackle IIO timeout. 

    ######## User configuration ##########
    # Channels to be captured e.g. [0]: 1chn
    device.rx_enabled_channels = ['voltage0']   # Noise test is primarily performed on AIN1-AIN2.
    
    # Minimum block of sample for test mode. Respective capture time is as follows
    # 1) Noise Test: 2 mintues.
    # 2) Fast 50Hz Test: 12 seconds.
    samples_block = 512
    
    # Total samples are received in multiple iterations or blocks
    # The samples needs to be captured in smaller blocks due to limitations
    # of buffer size (RAM) in the firmware and IIO client timeout factor.
    # Request to capture samples more than buffer size, will be ignored by firmware.
    # Large time taken to read the samples from device, may cause timeout on IIO client.
    ######################################

    # Get the channels count from user
    chn_count = len(device.rx_enabled_channels)

    # Store the rx buffer size and rx data type based on input channels
    device.rx_buffer_size = samples_block    # Size of the IIO buffer (buffer is submitted during call to rx() method)
    device._rx_data_type = np.int32     # size of ADC sample

    listener = keyboard.Listener(on_press=key_press_event)
    listener.start()

def read_user_inputs():
    global iterations
    global run_continuous
    global device
    global chn_count
    global samples_block
    global samples_count

    samples_count = int(input(f'Enter the number of samples to be captured in multiples of {samples_block} \n\
                                0: Unlimited \n\
                                <50-1000000>: '))

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
    global adc_voltage_sqr_avg
    global adc_voltage_avg
    global sample_volt

    adc_max_count_bipolar = 1 << 23
    offset = adc_max_count_bipolar
    adc_ref_voltage_nv = 2500000000.0
    gain = 1
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

    for i in range(0, len(data_arr)):
        sample_volt = (data_arr[i]- (adc_max_count_bipolar)) * (adc_ref_voltage_nv / ((adc_max_count_bipolar) * gain))
        # Get the sample square and add to previous sample square value
        adc_voltage_sqr_avg += pow(sample_volt, 2)
        adc_voltage_avg += sample_volt;
    writer.writerows(data_arr)
    print(device.to_volts(0,data_arr))
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
    rms_noise = float(math.sqrt(adc_voltage_sqr_avg) / samples_count)
    rms_voltage = float(adc_voltage_avg / samples_count)
    peak_to_peak_noise = 2 * math.sqrt(2) * rms_noise
    print("\nSamples Count: ", samples_count)
    print("\nRMS Noise (nV): ", rms_noise)
    print("\nRMS Voltage (nV): ", rms_voltage)
    print("\nRMS Average (nV): ", adc_voltage_sqr_avg) 
    print("\nPeak to Peak Noise (nV): ", peak_to_peak_noise)

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
