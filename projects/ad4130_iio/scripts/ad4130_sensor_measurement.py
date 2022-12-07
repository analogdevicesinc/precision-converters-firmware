import serial
from time import sleep
from pynput import keyboard
from adi import ad4130
from ad4130_xattr import *

# Delays in seconds
short_time = 0.1
long_time = 1
loadcell_settling_time = 2

# Global variables
weight_input = 0
loadcell_offset = [ 0 ]
loadcell_gain = [ 0 ]

# Power test channels defined in the firmware
POWER_TEST_V_AVDD_CHN = 0
POWER_TEST_V_IOVDD_CHN = 1
POWER_TEST_I_AVDD_CHN = 2
POWER_TEST_I_IOVDD_CHN = 3
POWER_TEST_V_AVSS_DGND_CHN = 4
POWER_TEST_V_REF_CHN = 5

# IIO Channel name and respective channel index mapping
chn_mappping = {
    "voltage0" : 0, "voltage1" : 1, "voltage2" : 2, "voltage3" : 3, "voltage4" : 4,
    "voltage5" : 5, "voltage6" : 6, "voltage7" : 7, "voltage8" : 8, "voltage9" : 9,
    "voltage10" : 10, "voltage11" : 11, "voltage12" : 12, "voltage13" : 13,
    "voltage14" : 14, "voltage15" : 15,

    "current0" : 0, "current1" : 1, "current2" : 2, "current3" : 3, "current4" : 4,
    "current5" : 5, "current6" : 6, "current7" : 7, "current8" : 8, "current9" : 9,
    "current10" : 10, "current11" : 11, "current12" : 12, "current13" : 13,
    "current14" : 14, "current15" : 15,

    "temp0" : 0, "temp1" : 1, "temp2" : 2, "temp3" : 3, "temp4" : 4,
    "temp5" : 5, "temp6" : 6, "temp7" : 7, "temp8" : 8, "temp9" : 9,
    "temp10" : 10, "temp11" : 11, "temp12" : 12, "temp13" : 13,
    "temp14" : 14, "temp15" : 15,
}

def key_press_event(key):
    global key_pressed
    key_pressed = True

def init_sensor_measurement():
    global device
    global demo_config
    global listener

    ######## User configuration ##########
    # Configure the backend for PC to IIOD interface
    uri = "serial:COM12,230400"  # For UART, baud rate must be same as set in the FW. COM port is physical Or VCOM.
    device_name = "ad4130-8"     # Name of the device must be same as set in the FW.
    ######################################

    # Create an IIO device context
    device = ad4130_xattr(uri, device_name)
    device._ctx.set_timeout(100000)
    device._rx_data_type = np.int32
    device._rx_stack_interleaved = True

    # Get current user device config from the firmware
    demo_config = device.demo_config
    print("\r\nDemo Config: {}\r\n".format(demo_config))

    listener = keyboard.Listener(on_press=key_press_event)
    listener.start()

def perform_loadcell_calibration(chn):
        global loadcell_offset
        global loadcell_gain
        global weight_input
        global device

        chn_indx = chn_mappping[chn.name]

        print("\r\nCalibrating Loadcell for channel {}".format(chn_indx))
        input("Please ensure no weight is applied on Loadcell and press enter to continue calibration ")
        print("Waiting to settle-down the Loadcell..")
        sleep(loadcell_settling_time)
        print("Performing Loadcell offset calibration..")
        chn.loadcell_offset_calibration = 'start_calibration'
        print("Loadcell offset calibration complete")
        loadcell_offset[chn_indx] = chn.loadcell_offset_calibration
        print("Loadcell offset: {}".format(loadcell_offset[chn_indx]))

        weight_input_done = False
        while weight_input_done is False:
            try:
                weight_input = int(input("\r\nApply the weight on loadcell and enter here (in grams): "))
                if (weight_input <= 0):
                    print("Please ensure weight is > 0!!")
                else:
                    weight_input_done = True
            except:
                print("Invalid input")

        print("Waiting to settle-down the Loadcell..")
        sleep(loadcell_settling_time)
        print("Performing load cell gain calibration..")
        chn.loadcell_gain_calibration = 'start_calibration'
        print("Load cell gain calibration complete")
        loadcell_gain[chn_indx] = chn.loadcell_gain_calibration
        print("Loadcell gain: {}".format(loadcell_gain[chn_indx]))

def perform_sensor_measurement():
    global device
    global key_pressed
    global loadcell_offset
    global loadcell_gain
    global weight_input

    # Loadcell must be calibrated before performing measurement
    if (demo_config == 'Loadcell'):
        for chn in device.channel:
            perform_loadcell_calibration(chn)

    weight = 0
    print("\r\n*** Press any key to stop the measurement ***\r\n")
    sleep(long_time)
    
    # Print the header
    header = ""
    for chn in device.channel:
        header = header + chn.name + ' '
    print(header)

    key_pressed = False
    while not key_pressed:
        result_str = ""
        for chn in device.channel:
            sleep(short_time)
            if (demo_config == 'Loadcell'):
                adc_raw = chn.raw
                try:
                    chn_indx = chn_mappping[chn.name]
                    weight = ((adc_raw - loadcell_offset[chn_indx]) * weight_input) / (loadcell_gain[chn_indx] - loadcell_offset[chn_indx])
                    result_str = result_str + str(round(weight,4)) + ' gram  '
                except:
                    print("\r\nInvalid measurement result. Please check device settings!!")
                    break
            elif (demo_config == 'ECG' or demo_config == 'Noise Test' or demo_config == 'Power Test' or demo_config == 'User Default'):
                adc_raw = chn.raw
                scale = chn.scale
                offset = chn.offset
                if (demo_config == 'Power Test'):
                    chn_indx = chn_mappping[chn.name]
                    if (chn_indx == POWER_TEST_I_AVDD_CHN or chn_indx == POWER_TEST_I_IOVDD_CHN):
                        current = ((adc_raw + offset) * scale)
                        result_str = result_str + str(round(current,4)) + 'mA  '
                    else:
                        voltage = ((adc_raw + offset) * scale) / 1000
                        result_str = result_str + str(round(voltage,4)) + 'V  '
                else:
                    voltage = ((adc_raw + offset) * scale) / 1000
                    result_str = result_str + str(round(voltage,4)) + 'V  '
            else:
                # Temperature sensors demo configs (RTD, Thermocouple and Thermistor)
                adc_raw = chn.raw
                scale = chn.scale
                temperature = (adc_raw * scale) / 1000
                result_str = result_str + str(round(temperature,4)) + 'C  '

        print(result_str)

def exit():
    global listener
    global device

    # Delete the objects
    del listener
    del device

def main():
    init_sensor_measurement()
    perform_sensor_measurement()
    exit()

if __name__ == "__main__":
    main()
