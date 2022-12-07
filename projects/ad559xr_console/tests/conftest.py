"""Define some fixtures to use in the project."""
import pytest
import serial
import re
from time import sleep


def pytest_addoption(parser):
    parser.addoption("--serialport", action="store", default=None)
    # these are included, but not used currently/anymore
    parser.addoption("--serialnumber", action="store", default=None)
    parser.addoption("--mountpoint", action="store", default=None)


@pytest.fixture(scope="session")
def serialport(pytestconfig):
    return pytestconfig.getoption("serialport")


@pytest.fixture(scope="session")
def serialnumber(pytestconfig):
    return pytestconfig.getoption("serialnumber")


@pytest.fixture(scope="session")
def mountpoint(pytestconfig):
    return pytestconfig.getoption("mountpoint")


@pytest.fixture(scope="session")
def serial_port(serialport):
    serp = serial.Serial(port=serialport, baudrate=230400, timeout=1)
    print("\nopening serial port", serp)

    yield serp

    # only do once all unit tests are complete
    print("\nclosing serial port", serp)
    serp.close()


@pytest.fixture(scope="function")
def target_reset(serial_port):
    # Need to reset the MCU, so it's in a known state before the test
    serial_port.send_break(1)

    # Allow time for system to reset before doing anything
    sleep(0.1)


@pytest.fixture(scope="function")
def check_enable_int_ref(serial_port):
    # Ensure internal reference is enabled after reset for each test
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

    # Press escape key to return to main menu
    serial_port.write(b'\x1b')
    sleep(0.1)

