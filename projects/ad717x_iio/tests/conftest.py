"""Define some fixtures to use in the project."""
import pytest
from time import sleep

firmware_name = 'ad717x_iio'

def pytest_addoption(parser):
    parser.addoption("--serialport", action="store", default=None)
    parser.addoption("--serial_com_type", action="store", default=None)
    parser.addoption("--device_name", action="store", default=None)
    parser.addoption("--platform_name", action="store", default=None)
    parser.addoption("--evb_interface", action="store", default=None)

@pytest.fixture(scope="session")
def serialport(pytestconfig):
    return pytestconfig.getoption("serialport")

@pytest.fixture(scope="session")
def serialcomtype(pytestconfig):
    return pytestconfig.getoption("serial_com_type")

@pytest.fixture(scope="session")
def devicename(pytestconfig):
    return pytestconfig.getoption("device_name")

@pytest.fixture(scope="session")
def platformname(pytestconfig):
    return pytestconfig.getoption("platform_name")
    
@pytest.fixture(scope="session")
def evbinterface(pytestconfig):
    return pytestconfig.getoption("evb_interface")

@pytest.fixture(scope="session")
def serial_port(serialport, serialcomtype, devicename, platformname, evbinterface):
        yield serialport

@pytest.fixture(scope="session")
def device_name(devicename):
    return devicename

@pytest.fixture(scope="session")
def serial_com_type(serialcomtype):
    return serialcomtype

@pytest.fixture(scope="function")
def target_reset(serial_port):
    # NA as serial port connection is opened through test example file
    
    # Allow time for system to reset before doing anything
    sleep(0.1)
