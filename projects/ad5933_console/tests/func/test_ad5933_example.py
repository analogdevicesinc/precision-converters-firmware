import pytest
import serial
from time import sleep
import re

# could make these fixtures if there is a need to share them more widely
short_time = 0.1
long_time = 0.2


def test_reset_action(serial_port, target_reset):
    """tests that when the Reset menu command is sent, the menu redraws as expected"""

    # Check for last line of menu
    expect_match_string = b'Make a selection...'
    expect_match_string_found = False

    # Read a large number of lines in case VT100 clear screen not used
    # port must have been opened with a timeout for this to work
    lines = serial_port.readlines() # .readlines(250)
   #  clear any pending input
    serial_port.reset_input_buffer()
    print("Reset item lines = ", lines)

    for line in lines:
        if expect_match_string in line:
            expect_match_string_found = True
            break

    assert expect_match_string_found

def test_read_temperature(serial_port, target_reset):
    """Read the temperature and compares vs sensible room temperature value"""

   #clear any pending input
    serial_port.reset_input_buffer()
    # Send the read temperature command
    serial_port.write(b'1\n')
    sleep(long_time)
    # Read a lines to see what ID is returned
    line = serial_port.readline()

    words = line.decode().split(":")
    words = words[1].split(" ")
    temperature = float(words[0])

    print(f"AD5933 reported temperature: {temperature}")

    assert temperature > 0
    assert temperature < 50