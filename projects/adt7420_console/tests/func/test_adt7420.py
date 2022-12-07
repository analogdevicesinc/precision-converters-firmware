import pytest
import re
from time import sleep

SLEEP_LONG = 1.5
SLEEP_MID = 0.5
SLEEP_SHORT = 0.1

device_setup_array =    [(b'1\n', b'ADT7320.*sensor.*selected'),
                        (b'2\n', b'ADT7420.*sensor.*selected!'),
                        (b'12\n', b'Invalid.*entry'),
                        (b'HI\n', b'Invalid.*entry')]

chip_setup_array =  [(b'1\n', b'Internal.*sensor.*selected'),
                    (b'2\n', b'External.*sensor.*selected'),
                    (b'12\n', b'Invalid.*entry.'), 
                    (b'HI\n', b'Invalid.*entry')]


@pytest.mark.parametrize('device_select, expected', device_setup_array)
def test_select_device(serial_port, device_select, expected):
    """tests selection of the interface (SPI/I2C) on device boot"""
    # Reset Device
    serial_port.send_break()
    sleep(SLEEP_LONG)
    serial_port.reset_input_buffer()

    # Select ADT7420
    serial_port.write(device_select)
    result = serial_port.read(40)

    assert re.search(expected, result)


@pytest.mark.parametrize('chip_select,expected', chip_setup_array)
def test_select_chip(serial_port, chip_select, expected, device_select_config):
    """Configure device to have it at external internal chip selection."""
    serial_port.send_break(SLEEP_SHORT)
    sleep(SLEEP_SHORT)
    serial_port.reset_input_buffer()
    serial_port.write(device_select_config)  # Tests for both I2C and SPI configs
    sleep(SLEEP_SHORT)
    serial_port.reset_input_buffer()

    # Select chip
    serial_port.write(chip_select)
    result = serial_port.read(50)

    assert re.search(expected, result)


def test_read_temp_valid(serial_port, device_setup):
    """Check if temp an be read and if it's within 10 and 49 degrees celsius"""
    # Selects read temp menu
    sleep(SLEEP_MID)
    serial_port.write(b'1\n')
    sleep(SLEEP_SHORT)

    expect_string = br'Current.?temperature.*[1-4][0-9]\..*C'

    result = serial_port.read(30)

    assert re.search(expect_string, result)


# Invalid check for character input commented as will always fail until changes are made to source code.
@pytest.mark.parametrize('set_resolution, expected', [(b'1\n', b'Set.*resolution.*13-bit'),
                                                      (b'2\n', b'Set.*resolution.*16-bit'),
                                                      (b'12\n', b'Invalid.*entry'),(b'HI\n', b'Invalid.*entry')])
def test_set_resolution(serial_port, device_setup, set_resolution, expected):
    # Select Set Resolution from main menu - Option 2
    serial_port.write(b'2\n')
    sleep(SLEEP_SHORT)
    serial_port.reset_input_buffer()

    serial_port.write(set_resolution)
    sleep(SLEEP_SHORT)
    result = serial_port.read(80)

    assert re.search(expected, result)


@pytest.mark.parametrize('menu_input, expected', [(b'12\n', b'Invalid.*entry'),
                                                  (b'HI\n', b'Invalid.*entry')])
def test_main_menu(serial_port, device_setup, menu_input, expected):
    """Testing menu response to invalid inputs"""
    serial_port.write(menu_input)
    result = serial_port.read(60)
    print("\nTHIS IS RESULT", result)
    assert re.search(expected, result)


op_mode_options = [(b'1\n', b'Read.*value.*0x0'), (b'2\n', b'Read.*value.*0x60'),
                   (b'3\n', b'Read.*value.*0x40'),
                   (b'4\n', b'Read.*value.*0x60'),
                   (b'13\n', b'Invalid.*entry')]


@pytest.mark.parametrize('op_mode, expected', op_mode_options)
def test_set_op_mode(serial_port, device_setup, op_mode, expected):
    # Select Set Operation Mode option in menu - Option 3
    serial_port.write(b'3\n')
    sleep(SLEEP_SHORT)
    serial_port.reset_input_buffer()

    serial_port.write(op_mode)
    immediate_read = serial_port.read(40)
    if op_mode == b'13\n':
        assert re.search(expected, immediate_read)
        return

    result = read_config_reg(serial_port, b'2\n')

    assert re.search(expected, result)


num_temp_samples = [(b'5\n', b'5\n', br'Sample.*5.*Temperature.*[1-4][0-9]\..*'),
                    (b'10\n', b'10\n', br'Sample.*10.*Temperature.*[1-4][0-9]\..*')]


@pytest.mark.parametrize('poll_temp, sample_rate, expected', num_temp_samples)
def test_poll_temp(serial_port, device_setup, poll_temp, sample_rate, expected):
    serial_port.write(b'4\n')
    sleep(SLEEP_SHORT)
    serial_port.reset_input_buffer()

    serial_port.write(poll_temp)
    sleep(SLEEP_SHORT)
    serial_port.reset_input_buffer()
    serial_port.write(sample_rate)
    sleep(SLEEP_SHORT)

    result = serial_port.read(500)

    assert re.search(expected, result)


register_options = [(b'1\n', b'Read.*value.*0x0'), (b'2\n', b'Read.*value.*0x0'),  # (b'3\n', b''), (b'4\n',),
                    (b'5\n', b'Read.*value.*0x4980'), (b'6\n', b'Read.*value.*0x5'),
                    (b'7\n', b'Read.*value.*0x2000'), (b'8\n', b'Read.*value.*0x500')]


@pytest.mark.parametrize('read_reg, expected', register_options)
def test_read_register(serial_port, device_setup, read_reg, expected):
    """This tests checks the default values of the device, ensuring they are correct.
            It doesn't check for an invalid input"""
    sleep(SLEEP_SHORT)
    serial_port.write(b'5\n')
    sleep(SLEEP_SHORT)
    serial_port.reset_input_buffer()

    serial_port.write(read_reg)
    sleep(SLEEP_SHORT)
    result = serial_port.read(40)

    assert re.search(expected, result)


def test_reset_interface(serial_port, device_setup):
    serial_port.write(b'6\n')
    expect_string = b'Resetting.*interface'

    result = serial_port.read(25)

    assert re.search(expect_string, result)


def test_write_to_reg(serial_port, device_setup):
    """This test just tests writing to one register - Setting the critical temp setpoint."""
    expect_string = b'Read.*value.*0xc80'
    serial_port.write(b'7\n')

    sleep(SLEEP_SHORT)
    serial_port.reset_input_buffer()
    serial_port.write(b'1\n')

    sleep(SLEEP_SHORT)
    serial_port.reset_input_buffer()
    serial_port.write(b'25\n')  # Sets value to 25 Degrees C
    serial_port.read(10)

    result = read_config_reg(serial_port, b'5\n')

    assert re.search(expect_string, result)


fault_queue_options = [(b'1\n', b'Read.*value.*0x0'), (b'2\n', b'Read.*value.*0x1'),
                       (b'3\n', b'Read.*value.*0x2'), (b'4\n', b'Read.*value.*0x3')]


@pytest.mark.parametrize('set_fault_queue, expected', fault_queue_options)
def test_set_fault_queue(serial_port, device_setup, set_fault_queue, expected):
    serial_port.write(b'8\n')

    sleep(SLEEP_SHORT)
    serial_port.reset_input_buffer()
    serial_port.write(set_fault_queue)

    result = read_config_reg(serial_port, b'2\n')

    assert re.search(expected, result)


int_ct_options = [(b'1\n', b'1\n', b'Read.*value.*0x0'), (b'1\n', b'2\n', b'Read.*value.*0xc'),
                  (b'2\n', b'1\n', b'Read.*value.*0x10'), (b'2\n', b'2\n', b'Read.*value.*0x1c')]


@pytest.mark.parametrize('mode_select, polarity_select, expected', int_ct_options)
def test_int_ct_settings(serial_port, device_setup, chip_select_config, mode_select, polarity_select, expected):
    serial_port.write(b'9\n')
    error_string = b'Feature.*available.*only.*for.*internal.*sensors'

    sleep(SLEEP_LONG)
    serial_port.reset_input_buffer()
    serial_port.write(mode_select)
    immediate_read = serial_port.read(200)

    if chip_select_config == b'2\n':
        assert re.search(error_string, immediate_read)
        return

    sleep(SLEEP_LONG)
    serial_port.reset_input_buffer()
    serial_port.write(polarity_select)

    result = read_config_reg(serial_port, b'2\n')

    assert re.search(expected, result)


def read_config_reg(serial_port, register_to_read):
    """Read registers to check if selection was set"""
    sleep(SLEEP_LONG)
    serial_port.reset_input_buffer()
    serial_port.write(b'5\n')
    sleep(SLEEP_SHORT)
    serial_port.reset_input_buffer()
    serial_port.write(register_to_read)
    result = serial_port.read(60)
    return result
