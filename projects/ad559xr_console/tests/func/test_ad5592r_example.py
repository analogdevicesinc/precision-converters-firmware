import pytest
import serial
from time import sleep
import re
import enum

# could make these fixtures if there is a need to share them more widely
short_time = 0.1
long_time = 0.2

def select_items(serial_port, items):
    for x in items:
        serial_port.write(x)
        sleep(short_time)


#===========================================================================================

def test_reset_action(serial_port, target_reset):
    """Test the device reset action"""

    expect_match_string = b'Successful'

    # clear any pending input
    serial_port.reset_input_buffer()

    # Send the Software Reset command, wait for input to arrive
    serial_port.write(b'Q')
    sleep(short_time)

    lines = serial_port.read(100)

    assert re.search(expect_match_string, lines)

    # Press any key to continue
    serial_port.write(b'!')
    sleep(short_time)

#===========================================================================================

def test_enable_internal_ref(serial_port, target_reset):
    """Checks if internal reference is enabled (Should be by default)"""

    expect_match_string = b'En Ref.*X'

    # clear any pending input
    serial_port.reset_input_buffer()

    # Send the General Settings Menu command, wait for input to arrive
    serial_port.write(b'S')
    sleep(0.1)

    lines = serial_port.read(100)

    if not re.search(expect_match_string, lines):
        # clear any pending input
        serial_port.reset_input_buffer()

        # Send the General Settings Menu command, wait for input to arrive
        serial_port.write(b'A')

        # Allow time for voltage to increase
        sleep(0.1)

        lines = serial_port.read(100)

        assert re.search(expect_match_string, lines)

    # Press escape key to return to main menu
    serial_port.write(b'\x1b')
    sleep(0.1)

#===========================================================================================

def test_read_die_temp(serial_port, target_reset, check_enable_int_ref):
    """Perform read of die temperature"""

    expect_match_string = b'Temperature: [1-4][0-9].*C'

    # clear any pending input
    serial_port.reset_input_buffer()

    # Send the die temp read command, wait for input to arrive
    serial_port.write(b'W')
    sleep(short_time)

    # Read lines
    lines = serial_port.read(100)
    print(lines)
    assert re.search(expect_match_string, lines)

    # Press any key to continue
    serial_port.write(b'!')
    sleep(short_time)

    # Press escape key to return to main menu
    serial_port.write(b'\x1b')

#===========================================================================================
# Collection of test the set channels as ADC and DAC. Voltages written and read from channels

def test_dac_adc_config(serial_port, target_reset, check_enable_int_ref):
    "Set Channel 0,1,2 as DAC and ADC"

    expect_match_string = b'0.*ADC.DAC.*\n.*1.*ADC.DAC.*\n.*2.*ADC.DAC'

    # clear any pending input
    serial_port.reset_input_buffer()
    sleep(short_time)

    # Send the Config Channel command, wait for input to arrive
    serial_port.write(b'A')
    sleep(short_time)

    # Select channels 0,1,2
    selection_list = {b'A', b'S', b'D'}
    select_items(serial_port, selection_list)

    # clear any pending input
    serial_port.reset_input_buffer()
    sleep(short_time)

    # Set selected channels as ADC + DAC
    serial_port.write(b'E')
    sleep(short_time)

    lines = serial_port.read(150)
    print(lines)

    assert re.search(expect_match_string, lines)

    # Press escape key to return to main menu
    serial_port.write(b'\x1b')
    sleep(short_time)

def test_dac_adc_set_dacs_to_midscale(serial_port, check_enable_int_ref):
    # "Set channels 0,1,2 to 1.25V "

    expect_match_string = b'0.*DAC.*1.2[4-6]V.*\n.*1.*DAC.*1.2[4-6]V.*\n.*2.*DAC.*1.2[4-6]V.*'

    # clear any pending input
    serial_port.reset_input_buffer()
    sleep(short_time)

    # Send the DAC Menu command, wait for input to arrive
    serial_port.write(b'D')
    sleep(short_time)

    # Select channels 0,1,2
    selection_list = {b'A', b'S', b'D'}
    select_items(serial_port, selection_list)

    # clear any pending input
    serial_port.reset_input_buffer()
    sleep(short_time)

    # Write to DAC output
    serial_port.write(b'Q')
    sleep(short_time)

    # Write 1.25V to DAC output
    serial_port.write(b'1.25\r\n')
    sleep(short_time)

    lines = serial_port.read(250)
    print(lines)

    assert re.search(expect_match_string, lines)

    # Press escape key to return to main menu
    serial_port.write(b'\x1b')
    sleep(short_time)


def test_dac_adc_read_sequence(serial_port, check_enable_int_ref):
    # Include channels set as ADCs in sequence and perform single sequence read

    expect_match_string = b'(1.2[4-6])'
    expected_match_found = False

    # clear any pending input
    serial_port.reset_input_buffer()
    sleep(short_time)

    # Send the ADC Menu command, wait for input to arrive
    serial_port.write(b'F')
    sleep(short_time)

    # Channels should already be selected from previous test

    # Send the command to include channels in readback sequence
    serial_port.write(b'Q')
    sleep(short_time)

    # clear any pending input
    serial_port.reset_input_buffer()
    sleep(short_time)

    # Run single ADC sequence, wait for input to arrive
    serial_port.write(b'W')
    sleep(short_time)

    lines = serial_port.read(100)

    # Get number of times expected voltage value is found
    result = re.findall(expect_match_string, lines)

    # If found the expected number of times assert True
    if len(result) is 3:
        expected_match_found = True

    assert expected_match_found

    # Press any key to continue
    serial_port.write(b'!')
    sleep(short_time)

    # Press escape key to return to main menu
    serial_port.write(b'\x1b')
    sleep(short_time)


#===========================================================================================
# Collection of tests to test GPIO functionality.
# NOTE: These tests require channel 5 and 6 to be connected on the AD5592R/93R

def test_gpio_gpo_config(serial_port, target_reset):
    # Configure channels 5 as GPO and toggle high

    expect_match_strings = b'5.*Out'

    # clear any pending input
    serial_port.reset_input_buffer()
    sleep(short_time)

    # Send GPIO menu command
    serial_port.write(b'G')
    sleep(short_time)

    # Select Channel 5
    serial_port.write(b'H')
    sleep(short_time)

    # clear any pending input
    serial_port.reset_input_buffer()
    sleep(short_time)

    # Set channel 5 as GPO
    serial_port.write(b'X')
    sleep(long_time)

    lines = serial_port.read(150)

    assert re.search(expect_match_strings, lines)

def test_gpio_gpo_high(serial_port):
    # Toggle Channel 5 GPO High

    expect_match_strings = b'5.*Out.*High'

    # clear any pending input
    serial_port.reset_input_buffer()
    sleep(short_time)

    # Select Channel 5
    serial_port.write(b'H')
    sleep(short_time)

    # clear any pending input
    serial_port.reset_input_buffer()
    sleep(short_time)

    # Toggle channel 5 GPO as High
    serial_port.write(b'C')
    sleep(long_time)

    lines = serial_port.read(150)

    assert re.search(expect_match_strings,lines)


def test_gpio_gpi_config(serial_port):
    # Configure channel 6 as a GPI

    expect_match_string = b'6.*In.*High'

    # clear any pending input
    serial_port.reset_input_buffer()
    sleep(short_time)

    # Select Channel 5
    serial_port.write(b'J')
    sleep(short_time)

    # clear any pending input
    serial_port.reset_input_buffer()
    sleep(short_time)

    # Select as GPI
    serial_port.write(b'Z')
    sleep(short_time)

    lines = serial_port.read(150)

    assert re.search(expect_match_string, lines)


#===========================================================================================
