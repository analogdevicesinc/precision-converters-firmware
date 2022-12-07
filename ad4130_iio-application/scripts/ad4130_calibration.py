import serial
from time import sleep
from adi import ad4130
from ad4130_xattr import *

# Delays in second
short_time = 0.2

# Calibration type identifiers
internal_calibration = '1'
system_calibration = '2'

# Analog input mapping as configured in the firmware
ain_mapping = {
    "User Default" :  [ 'AIN0-AVSS', 'AIN1-AVSS', 'AIN2-AVSS', 'AIN3-AVSS', 
                        'AIN4-AVSS', 'AIN5-AVSS', 'AIN6-AVSS', 'AIN7-AVSS',
                        'AIN8-AVSS', 'AIN9-AVSS', 'AIN10-AVSS', 'AIN11-AVSS', 
                        'AIN12-AVSS', 'AIN13-AVSS', 'AIN14-AVSS', 'AIN15-AVSS'
                      ],
    "Thermistor" :    [ 'AIN4-AIN5' ],
    "Thermocouple" :  [ 'AIN2-AIN3', 'AIN4-AIN5' ],
    "2-Wire RTD" :    [ 'AIN2-AIN3' ],
    "3-Wire RTD" :    [ 'AIN2-AIN3' ],
    "4-Wire RTD" :    [ 'AIN2-AIN3' ],
    "Loadcell"   :    [ 'AIN5-AIN6' ],
    "ECG"        :    [ 'AIN11-AIN14' ],
    "Noise Test" :    [ 'AIN0-AIN1' ],
}

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

def init_calibration():
    global device
    global demo_config

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

def get_calibration_status(calibration_type, chn, chn_index):
    global gain_before_calib, gain_after_calib
    global offset_before_calib, offset_after_calib
    global calibration_status

    if (calibration_type == system_calibration):
        calib_status = chn.system_calibration
    else:
        calib_status = chn.internal_calibration

    gain_before_calib = calib_status[0:8]
    gain_after_calib = calib_status[8:16]
    offset_before_calib = calib_status[16:24]
    offset_after_calib = calib_status[24:32]
    calibration_status = calib_status[32:]

def perform_calibration():

    if (demo_config == "Power Test"):
        # Power test uses internal ADC channels, on which calibration can't be performed
        print("Invalid demo mode config. Calibration can't be performed on internal ADC channels!!")
        return

    # Select calibration type
    calibration_type = input("\r\nSelect Calibration Type:\r\n\
                            {}. Internal Calibration\r\n\
                            {}. System Calibration\r\n".format(internal_calibration, system_calibration))
    if (calibration_type > system_calibration):
        print("Invalid Input!!")
        return

    # Perform calibration for all channels
    for chn in device.channel:
        chn_index = chn_mappping[chn.name]
        print("-------------------------------------------")
        print("Calibrating channel {} ".format(chn_index))

        if (calibration_type == system_calibration):
            # Perform zero-scale (offset) system calibration
            val = input("Apply zero-scale voltage between {} and press enter".format(ain_mapping[demo_config][chn_index]))
            chn.system_calibration = "start_calibration"
            sleep(short_time)
            get_calibration_status(calibration_type, chn, chn_index)
            print("Offset (before calibration): 0x{}".format(offset_before_calib))
            print("Offset (after calibration): 0x{}".format(offset_after_calib))
            if (calibration_status == "calibration_done"):
                print("System offset calibration successfull..\r\n")
            else:
                print("System offset calibration failed!!\r\n")

            # Perform full-scale (gain) system calibration
            val = input("Apply full-scale voltage between {} and press enter".format(ain_mapping[demo_config][chn_index]))
            chn.system_calibration = "start_calibration"
            sleep(short_time)
            get_calibration_status(calibration_type, chn, chn_index)
            print("Gain (before calibration): 0x{}".format(gain_before_calib))
            print("Gain (after calibration): 0x{}".format(gain_after_calib))
            if (calibration_status == "calibration_done"):
                print("System gain calibration successfull..\r\n")
            else:
                print("System gain calibration failed!!\r\n")
        else:
            # Perform full-scale (gain) internal calibration
            chn.internal_calibration = "start_calibration"
            sleep(short_time)
            get_calibration_status(calibration_type, chn, chn_index)
            print("Gain (before calibration): 0x{}".format(gain_before_calib))
            print("Gain (after calibration): 0x{}".format(gain_after_calib))
            if (calibration_status == "calibration_done"):
                print("Internal gain calibration successfull..\r\n")
            elif (calibration_status == "calibration_skipped"):
                print("Internal gain calibration skipped due to PGA=1\r\n")
            else:
                print("Internal gain calibration failed..\r\n")

            # Perform zero-scale (offset) internal calibration
            chn.internal_calibration = "start_calibration"
            sleep(short_time)
            get_calibration_status(calibration_type, chn, chn_index)
            print("Offset (before calibration): 0x{}".format(offset_before_calib))
            print("Offset (after calibration): 0x{}".format(offset_after_calib))
            if (calibration_status == "calibration_done"):
                print("Internal offset calibration successfull..\r\n")
            else:
                print("Internal offset calibration failed!!\r\n")

def exit():
    global device

    # Delete the objects
    del device

def main():
    init_calibration()
    perform_calibration()
    exit()

if __name__ == "__main__":
    main()