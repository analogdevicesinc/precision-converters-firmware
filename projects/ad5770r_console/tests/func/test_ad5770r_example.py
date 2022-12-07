from time import sleep

# could make these fixtures if there is a need to share them more widely
short_time = 0.1
long_time = 0.2

ESCAPE_KEY_CODE = b"\x1B"
# ===========================================================================================


def send_stdin_check_stdout(serial_port, serial_out_pretest, serial_out_test, expect_strings, lines_to_try_read=2000):
    """ send a string of characters out, and then scrape the output for expected strings.  All strings
    must be found for this function to pass the test"""

    expect_match_string_found = []

    # Send the user input before we do actual test
    for char in serial_out_pretest:
        serial_port.write(char)
        sleep(short_time)

    # clear any pending input
    serial_port.reset_input_buffer()
    lines = []

    # Send the Reset Menu command, wait for input to arrive
    for char in serial_out_test:
        serial_port.write(char)
        sleep(long_time)

        # Read a large number of lines in case VT100 clear screen not used
        # port must have been opened with a timeout for this to work
        lines = lines + serial_port.readlines(lines_to_try_read)

    # Check for each of the expect strings
    for match_string in expect_strings:
        expect_match_string_found.append(False)
        for line in lines:
            if match_string in line:
                expect_match_string_found[-1] = True
                break

    # Create this variable, to aid with debug when tests fail to know which strings were not found
    expect_stings_found_results = list(zip(expect_strings, expect_match_string_found))

    assert all(elem == True for elem in expect_match_string_found)

# ===========================================================================================


def test_reset_action(serial_port, target_reset):
    """Test the device reset action"""

    serial_out_pretest = [b'G', ESCAPE_KEY_CODE]  # go into a sub menu to increment the scratchpad register
    serial_out_test = [b'R', b'!']
    expect_match_strings = [b'Software Reset Succeeded', b'AD5770R Console App', b'Scratchpad = 0x00',
                            b'Scratchpad = 0x01']

    send_stdin_check_stdout(serial_port, serial_out_pretest, serial_out_test, expect_match_strings)

# ===========================================================================================


def test_device_init_action(serial_port, target_reset):
    """Test the device initialization action, and that it returns to main menu"""

    serial_out_pretest = []
    serial_out_test = [b'I', b'!']
    expect_match_strings = [b'ad5770r successfully initialized', b'AD5770R Console App']

    send_stdin_check_stdout(serial_port, serial_out_pretest, serial_out_test, expect_match_strings)

# ===========================================================================================


def test_device_remove_action(serial_port, target_reset):
    """Test the device remove action, and that it returns to main menu"""

    serial_out_pretest = []
    serial_out_test = [b'X', b'!']
    expect_match_strings = [b'Press any key to continue...', b'AD5770R Console App']

    send_stdin_check_stdout(serial_port, serial_out_pretest, serial_out_test, expect_match_strings)

# ===========================================================================================


def test_general_config_action_1(serial_port, target_reset):
    """Test the general configuration action with one set of settings, and that it returns to main menu
    Assumes the user config defaults are set as follows:
        Ref Resistor: External          Ref Voltage: 2
        Alarms  BgCRC Msk: 0    IRef: 0 neg: 0  OT: 0
                T Warn: 0       BgCRC En: 0     T Shdn: 0       OD: 0"""

    serial_out_pretest = [b'G', b'R', b'F', b'1', b'3', b'5']
    serial_out_test = [b'7', ESCAPE_KEY_CODE]

    expect_match_strings = [b'Ref Resistor: Internal',
                            b'Ref Voltage: 3',
                            b'BgCRC Msk: 1',
                            b'IRef: 0',
                            b'neg: 1',
                            b'OT: 0',
                            b'T Warn: 1',
                            b'BgCRC En: 0',
                            b'T Shdn: 1',
                            b'OD: 0',
                            b'AD5770R Console App']

    send_stdin_check_stdout(serial_port, serial_out_pretest, serial_out_test, expect_match_strings)

# ===========================================================================================


def test_general_config_action_2(serial_port, target_reset):
    """Test the general configuration action with the alternate set of settings, and that it returns to main menu
    Assumes the user config defaults are set as follows:
        Ref Resistor: External          Ref Voltage: 2
        Alarms  BgCRC Msk: 0    IRef: 0 neg: 0  OT: 0
                T Warn: 0       BgCRC En: 0     T Shdn: 0       OD: 0"""
    serial_out_pretest = [b'G', b'S', b'0', b'2', b'4']
    serial_out_test = [b'6', ESCAPE_KEY_CODE]

    expect_match_strings = [b' General Configuration',
                            b'Ref Resistor: External',
                            b'Ref Voltage: 1',
                            b'BgCRC Msk: 0',
                            b'IRef: 1',
                            b'neg: 0',
                            b'OT: 1',
                            b'T Warn: 0',
                            b'BgCRC En: 1',
                            b'T Shdn: 0',
                            b'OD: 1',
                            b'AD5770R Console App']

    send_stdin_check_stdout(serial_port, serial_out_pretest, serial_out_test, expect_match_strings)

# ===========================================================================================


def test_monitor_setup_action_1(serial_port, target_reset):
    """Test the monitor setup action, for one config and that it returns to main menu.
    Assumes the user config is:
        Monitor: Current Ch 3   Buffer: Off     IB_Ext: On"""

    serial_out_pretest = [b'M', b'W', b'M', b'4']
    serial_out_test = [b'X', ESCAPE_KEY_CODE]
    expect_match_strings = [b'Monitor Setup',
                            b'Monitor: Voltage Ch 4',
                            b'Buffer: On',
                            b'IB_Ext: Off',
                            b'AD5770R Console App']

    send_stdin_check_stdout(serial_port, serial_out_pretest, serial_out_test, expect_match_strings)

# ===========================================================================================


def test_monitor_setup_action_2(serial_port, target_reset):
    """Test the monitor setup action, for a second config and that it returns to main menu.
    Assumes the user config is:
        Monitor: Current Ch 3   Buffer: Off     IB_Ext: On"""

    serial_out_pretest = [b'M', b'R', b'X', b'1']
    serial_out_test = [b'X', ESCAPE_KEY_CODE]
    expect_match_strings = [b'Monitor Setup',
                            b'Monitor: Temperature',
                            b'Buffer: Off',
                            b'IB_Ext: On',
                            b'AD5770R Console App']

    send_stdin_check_stdout(serial_port, serial_out_pretest, serial_out_test, expect_match_strings)

# ===========================================================================================


def test_dac_config_action_1(serial_port, target_reset):
    """Test the DAC configuration action with one group of settings, and that it returns to main menu.
    Assumes default user config is:
    Ch Configs - en0: 1 sink0: 0  en1: 1  en2: 1  en3: 1  en4: 1  en5: 1"""

    serial_out_pretest = [b'C', b'0', b'2', b'4']
    serial_out_test = [b'S', ESCAPE_KEY_CODE]
    expect_match_strings = [b'DAC Channel Configuration',
                            b'en0: 0',
                            b'sink0: 1',
                            b'en1: 1',
                            b'en2: 0',
                            b'en3: 1',
                            b'en4: 0',
                            b'en5: 1',
                            b'AD5770R Console App']

    send_stdin_check_stdout(serial_port, serial_out_pretest, serial_out_test, expect_match_strings)

# ===========================================================================================


def test_dac_config_action_2(serial_port, target_reset):
    """Test the DAC configuration action with a second group of settings, and that it returns to main menu.
    Assumes default user config is:
    Ch Configs - en0: 1 sink0: 0  en1: 1  en2: 1  en3: 1  en4: 1  en5: 1"""

    serial_out_pretest = [b'C', b'S', b'1', b'3', b'5']
    serial_out_test = [b'S', ESCAPE_KEY_CODE]
    expect_match_strings = [b'DAC Channel Configuration',
                            b'en0: 1',
                            b'sink0: 0',
                            b'en1: 0',
                            b'en2: 1',
                            b'en3: 0',
                            b'en4: 1',
                            b'en5: 0',
                            b'AD5770R Console App']

    send_stdin_check_stdout(serial_port, serial_out_pretest, serial_out_test, expect_match_strings)

# ===========================================================================================


def test_dac_operations_action_input_swldac_1(serial_port, target_reset):
    """Test the DAC Operations SW LDAC for even channels, and that it returns to main menu"""

    serial_out_pretest = [b'D',
                          b'Q', b'1234\n', b'E', b'2345\n', b'T', b'3456\n',  # set input regs
                          b'0', b'2', b'4', b'U',    # set SW ldac and update DAC reg from input
                          b'Q', b'abc\n', b'E', b'bcd\n', b'T', b'def\n'] # set input regs to different value
    serial_out_test = [b'2', b'2', ESCAPE_KEY_CODE]     # the two writes of '2' are dummy, intended to have no effect
    expect_match_strings = [b'DAC Operations',
                            b'DAC: 0x1234',
                            b'DAC: 0x2345',
                            b'DAC: 0x3456',
                            b'ch0: 1',
                            b'ch1: 0',
                            b'ch2: 1',
                            b'ch3: 0',
                            b'ch4: 1',
                            b'ch5: 0',
                            b'Ch 0 - Input: 0x0ABC',
                            b'Ch 2 - Input: 0x0BCD',
                            b'Ch 4 - Input: 0x0DEF',
                            b'AD5770R Console App']

    send_stdin_check_stdout(serial_port, serial_out_pretest, serial_out_test, expect_match_strings)

# ===========================================================================================


def test_dac_operations_action_input_swldac_2(serial_port, target_reset):
    """Test the DAC Operations SW LDAC for odd channels, and that it returns to main menu"""

    serial_out_pretest = [b'D',
                          b'W', b'23A\n', b'R', b'34A\n', b'Y', b'45A\n',  # set input regs
                          b'1', b'3', b'5', b'U',    # set SW ldac and update DAC reg from input
                          b'W', b'ab8\n', b'R', b'bc8\n', b'Y', b'de8\n'] # set input regs to different value
    serial_out_test = [b'1', b'1', ESCAPE_KEY_CODE]    # the two writes of '1' are dummy, intended to have no effect
    expect_match_strings = [b'DAC Operations',
                            b'DAC: 0x023A',
                            b'DAC: 0x034A',
                            b'DAC: 0x045A',
                            b'ch0: 0',
                            b'ch1: 1',
                            b'ch2: 0',
                            b'ch3: 1',
                            b'ch4: 0',
                            b'ch5: 1',
                            b'Ch 1 - Input: 0x0AB8',
                            b'Ch 3 - Input: 0x0BC8',
                            b'Ch 5 - Input: 0x0DE8',
                            b'AD5770R Console App']

    send_stdin_check_stdout(serial_port, serial_out_pretest, serial_out_test, expect_match_strings)

# ===========================================================================================


def test_dac_operations_action_dac_output_1(serial_port, target_reset):
    """Test the DAC Operations DAC output menu items for even channels, and that it returns to main menu"""

    serial_out_pretest = [b'D',
                          b'A', b'220\n', b'D', b'442\n', b'G', b'664\n',  # set DAC regs
                          b'1']
    serial_out_test = [b'1', b'J', b'!', ESCAPE_KEY_CODE]
    expect_match_strings = [b'DAC Operations',
                            b'Ch 0 - Input: 0x0220',
                            b'Ch 2 - Input: 0x0442',
                            b'Ch 4 - Input: 0x0664',
                            b'DAC: 0x0220',
                            b'DAC: 0x0442',
                            b'DAC: 0x0664',
                            b'ch0: 0',
                            b'ch1: 0',
                            b'ch2: 0',
                            b'ch3: 0',
                            b'ch4: 0',
                            b'ch5: 0',
                            b'HW LDAC toggled',
                            b'AD5770R Console App']

    send_stdin_check_stdout(serial_port, serial_out_pretest, serial_out_test, expect_match_strings)

# ===========================================================================================


def test_dac_operations_action_dac_output_2(serial_port, target_reset):
    """Test the DAC Operations DAC output menu items for odd channels, and that it returns to main menu"""

    serial_out_pretest = [b'D',
                          b'S', b'331\n', b'F', b'553\n', b'H', b'775\n',    # set DAC regs
                          b'1']
    serial_out_test = [ b'1', b'J', b'!', ESCAPE_KEY_CODE]
    expect_match_strings = [b'DAC Operations',
                            b'Ch 1 - Input: 0x0331',
                            b'Ch 3 - Input: 0x0553',
                            b'Ch 5 - Input: 0x0775',
                            b'DAC: 0x0331',
                            b'DAC: 0x0553',
                            b'DAC: 0x0775',
                            b'ch0: 0',
                            b'ch1: 0',
                            b'ch2: 0',
                            b'ch3: 0',
                            b'ch4: 0',
                            b'ch5: 0',
                            b'HW LDAC toggled',
                            b'AD5770R Console App']

    send_stdin_check_stdout(serial_port, serial_out_pretest, serial_out_test, expect_match_strings)

# ===========================================================================================