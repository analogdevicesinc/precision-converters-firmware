"""Define some fixtures to use in the project."""
import pytest
import serial
from time import sleep
import serial.tools.list_ports

ADI_USB_VID = 0x0456
ADI_USB_PID = 0xb66c
firmware_name = 'AD4696_IIO_APPLICATION'

def pytest_addoption(parser):
    parser.addoption("--serialport", action="store", default=None)
    parser.addoption("--serial_com_type", action="store", default=None)
    parser.addoption("--device_name", action="store", default=None)
    parser.addoption("--platform_name", action="store", default=None)

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
def serial_port(serialport, serialcomtype, devicename, platformname):
    sleep(3)
    if (serialcomtype == 'USE_VIRTUAL_COM_PORT'):
        vcom_serial_number = firmware_name + '_' + devicename + '_' + platformname
        for port, desc, hwid in sorted(serial.tools.list_ports.comports()):
            print("{}: {} [{}]".format(port, desc, hwid))
        # Get the COM port number corresponding to virtualCOM port parameters
        for vcom_serial_port in serial.tools.list_ports.comports():
            if (vcom_serial_port.vid == ADI_USB_VID and vcom_serial_port.pid == ADI_USB_PID and vcom_serial_port.serial_number == vcom_serial_number):
                break
        yield vcom_serial_port.device
    else:
        for uart_serial_port in serial.tools.list_ports.comports():
            if(uart_serial_port == serialport):
                break
        yield uart_serial_port.device

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
