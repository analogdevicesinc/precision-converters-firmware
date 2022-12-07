"""Define some fixtures to use in the project."""
import pytest
import serial
from time import sleep

SLEEP_LONG = 1.5
SLEEP_MID = 0.5
SLEEP_SHORT = 0.1


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
    # note this is now using serial break instead, as that works on STM32 boards
    serial_port.send_break(1)
    # os.system(r"..\..\scripts\stm32_reset.bat " + serialnumber)

    # Allow time for system to reset before doing anything
    sleep(0.1)

inputs = [b'1\n', b'2\n']


@pytest.fixture(params=inputs)
def device_select_config(request):
    return request.param


@pytest.fixture(params=inputs)
def chip_select_config(request):
    return request.param


@pytest.fixture(scope="function")
def device_setup(serial_port, device_select_config, chip_select_config):
    """Gets device to the menu prompt -
        fixture allows it to be called by any function that requires to start at main menu"""
    device_reset(serial_port)
    sleep(SLEEP_SHORT)
    # Clears serial line
    serial_port.reset_input_buffer()

    # Selects SPI and then I2C
    serial_port.write(device_select_config)
    # Clear serial line again
    sleep(SLEEP_SHORT)

    serial_port.reset_input_buffer()
    # Selects Internal or External Chip
    serial_port.write(chip_select_config)

    sleep(SLEEP_SHORT)
    serial_port.reset_input_buffer()
    # Main menu commands now available for selection

def device_reset(serial_port):
    serial_port.send_break(SLEEP_SHORT)
