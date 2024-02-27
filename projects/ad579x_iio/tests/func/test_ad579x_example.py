import sys
sys.path.insert(0,'../../scripts')
import libm2k as l
import pytest
from time import sleep
from ad579x_xattr import *

# Global variables
iio_device = {'DEV_AD5780': 'ad5780'}


def test_ad579x(serial_port, device_name, serial_com_type, target_reset):
    uri_str = "serial:" + serial_port + ",230400"

    # Allow VCOM connection to establish upon power-up
    sleep(3)

    # Create a new instance of AD579x class which creates
    # a device context as well for IIO interface
    dev = ad579x_xattr(uri_str, iio_device[device_name])

    # Get the DAC resolution
    dac_res = dev.output_bits[0]

    # Form a list with dac codes at full scale, mid-scale and 25% scale
    max_code = pow(2, dac_res)
    dac_codes = [max_code - 1, max_code / 2 - 1, max_code / 4 - 1]

    # Power up the channel
    dev.channel[0].powerdown = 0
    dev.coding_select = "offset_binary"

    # ************ Read the analog output voltage at channel 0 *************

    channel = 0
    ctx = l.m2kOpen()
    assert ctx, "Connection Error: No ADALM2000 device available/connected to your PC."

    ain = ctx.getAnalogIn()

    # Prevent bad initial config
    ain.reset()
    ctx.calibrateADC()
    ain.enableChannel(channel, True)

    # Iterate over the defined test values and validate the outputs
    for val in dac_codes:
        dev.channel[0].raw = val
        val = dev.channel[0].raw
        scale = dev.channel[0].scale
        offset = dev.channel[0].offset
        v_calculated = (val + offset) * scale / 1000
        v_measured = ain.getVoltage()[channel]
        assert (v_calculated - 0.15 <= v_measured <= v_calculated + 0.15), "Error writing DAC codes!!"

    l.contextClose(ctx)
