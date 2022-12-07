"""Define some fixtures to use in the project."""
import pytest
import serial
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
def sensor_select(sensor_cmd):
    # Inputs command correponding to each sensor
    return sensor_cmd
