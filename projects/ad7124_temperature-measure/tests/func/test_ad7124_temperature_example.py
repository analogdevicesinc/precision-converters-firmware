from time import sleep
import re

# could make these fixtures if there is a need to share them more widely
short_time = 0.1
long_time = 0.2

ESCAPE_KEY_CODE = b"\x1B"
NUM_OF_NTC_SENSORS = 4
MIN_EXP_NTC_TEMPERATURE = 20.0
MAX_EXP_NTC_TEMPERATURE = 30.0
# ===========================================================================================

def send_stdin_check_stdout(serial_port, serial_out_pretest, serial_out_test, expect_reg_expr_strings, lines_to_try_read=2000):
    """ send a string of characters out, and then scrape the output for expected string expression.  All string expressions
    must be found for this function to pass the test"""

    expect_match_reg_expr_string_found = []

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

    # Check for each of the expect string expressions
    for match_expr_string in expect_reg_expr_strings:
        expect_match_reg_expr_string_found.append(False)
        for line in lines:
            if re.search(match_expr_string, line.decode()):
                expect_match_reg_expr_string_found[-1] = True
                break

    # Create this variable, to aid with debug when tests fail to know which string expressions were not found
    expect_stings_found_results = list(zip(expect_reg_expr_strings, expect_match_reg_expr_string_found))
    assert all(elem == True for elem in expect_match_reg_expr_string_found)

    return lines

# ===========================================================================================

def perform_thermistor_measurement(serial_port):
    """Perform the thermistor measurement"""

    # Verify the temperature (averaged) measurement results
    serial_out_pretest = []
    serial_out_test = [b'A']
    expect_match_reg_exps = [r"\s*NTC1\s*NTC2\s*NTC3\s*NTC4\s*"]

    lines = send_stdin_check_stdout(serial_port, serial_out_pretest, serial_out_test, expect_match_reg_exps)

    # Decode the NTC temperature values read from serial buffer
    expect_match_reg_exps = r"\s*(\d*\.\d*)\s*(\d*\.\d*)\s*(\d*\.\d*)\s*(\d*\.\d*)\s*"
    for line in lines:
        p = re.search(expect_match_reg_exps, line.decode())
        if p:
            expect_match_reg_exps_found = True
            break

    assert expect_match_reg_exps_found, "Error in getting NTC temperature values"

    # Verify the temperature values for correctness
    for sensor in range(1,NUM_OF_NTC_SENSORS+1):
        temperature = p.group(sensor)
        assert (MIN_EXP_NTC_TEMPERATURE <= float(temperature) <= MAX_EXP_NTC_TEMPERATURE), "Error in NTC temperature measurement!!"
        print('\nNTC {0}: {1}'.format(sensor, temperature))

# ===========================================================================================

def test_main_menu(serial_port, target_reset):
    """Test the main menu after power-up"""

    serial_out_pretest = []
    serial_out_test = [b'R']
    expect_match_reg_exps = [r"Current Config:\s*RESET,\s*Active Device:\s*AD7124-4", 
                            r"\[A\]\s*2-Wire RTD", 
                            r"\[B\]\s*3-Wire RTD",
                            r"\[C\]\s*4-Wire RTD",
                            r"\[D\]\s*Thermocouple",
                            r"\[E\]\s*Thermistor",
                            r"\[F\]\s*Calibrate ADC",
                            r"\[R\]\s*Reset Config"
                            ]

    send_stdin_check_stdout(serial_port, serial_out_pretest, serial_out_test, expect_match_reg_exps)

# ===========================================================================================

def test_thermistor_measurement_without_calibration(serial_port, target_reset):
    """Perform the thermistor measurement without calibration"""
    print("\nNTC Measurement Without Calibration => ")

    # Enable all NTC channels and verify the channel configurations
    serial_out_pretest = []
    serial_out_test = [b'E', b'2', b'3', b'4']
    expect_match_reg_exps = [r'NTC1\s*0\s*AIN0\s*AIN1\s*Y', 
                            r'NTC2\s*1\s*AIN2\s*AIN3\s*Y', 
                            r'NTC3\s*2\s*AIN4\s*AIN5\s*Y',
                            r'NTC4\s*3\s*AIN6\s*AIN7\s*Y'
                            ]

    send_stdin_check_stdout(serial_port, serial_out_pretest, serial_out_test, expect_match_reg_exps)

    perform_thermistor_measurement(serial_port)

# ===========================================================================================

def test_calibrate_and_perform_thermistor_measurement(serial_port, target_reset):
    """Perform the thermistor measurement after calibration"""
    print("\nNTC Measurement After Calibration => ")

    # Enable all NTC channels and then perform internal calibration
    serial_out_pretest = []
    serial_out_test = [b'E', b'2', b'3', b'4', ESCAPE_KEY_CODE, b'F', b'I']
    expect_match_reg_exps = [r'\s*Calibrating Channel 0 =>\s*', 
                            r'\s*Calibration done\s*', 
                            r'\s*Calibrating Channel 1 =>\s*', 
                            r'\s*Calibration done\s*',
                            r'\s*Calibrating Channel 2 =>\s*', 
                            r'\s*Calibration done\s*',
                            r'\s*Calibrating Channel 3 =>\s*', 
                            r'\s*Calibration done\s*'
                            ]

    send_stdin_check_stdout(serial_port, serial_out_pretest, serial_out_test, expect_match_reg_exps)
    print("Calibration Successful...")

    # Enter back into thermistor submenu and measure the temperature
    serial_port.write(b'!')
    sleep(short_time)

    serial_port.write(ESCAPE_KEY_CODE)
    sleep(short_time)

    serial_port.write(b'E')
    sleep(short_time)

    perform_thermistor_measurement(serial_port)

# ===========================================================================================

def test_2_wire_rtd_menu(serial_port, target_reset):
    """Verify the 2-wire RTD configurations"""

    serial_out_pretest = []
    serial_out_test = [b'A']
    expect_match_reg_exps = [r'RTD1\s*0\s*AIN0\s*AIN2\s*AIN3', 
                            r'RTD2\s*1\s*AIN1\s*AIN4\s*AIN5'
                            ]

    send_stdin_check_stdout(serial_port, serial_out_pretest, serial_out_test, expect_match_reg_exps)

# ===========================================================================================

def test_3_wire_rtd_menu(serial_port, target_reset):
    """Verify the 3-wire RTD configurations"""

    serial_out_pretest = []
    serial_out_test = [b'B']
    expect_match_reg_exps = [r'RTD1\s*0\s*AIN0\s*AIN1\s*AIN2\s*AIN3', 
                            r'RTD2\s*1\s*AIN6\s*AIN7\s*AIN4\s*AIN5'
                            ]

    send_stdin_check_stdout(serial_port, serial_out_pretest, serial_out_test, expect_match_reg_exps)

# ===========================================================================================

def test_4_wire_rtd_menu(serial_port, target_reset):
    """Verify the 4-wire RTD configurations"""

    serial_out_pretest = []
    serial_out_test = [b'C']
    expect_match_reg_exps = [r'RTD1\s*0\s*AIN0\s*AIN2\s*AIN3', 
                            r'RTD2\s*1\s*AIN1\s*AIN4\s*AIN5'
                            ]

    send_stdin_check_stdout(serial_port, serial_out_pretest, serial_out_test, expect_match_reg_exps)

# ===========================================================================================

def test_thermocouple_menu(serial_port, target_reset):
    """Verify the thermocouple configurations"""

    serial_out_pretest = []
    serial_out_test = [b'D']
    expect_match_reg_exps = [r'TC1\s*0\s*\-\s*AIN2\s*AIN3\s*', 
                            r'TC2\s*1\s*\-\s*AIN6\s*AIN7\s*'
                            ]

    send_stdin_check_stdout(serial_port, serial_out_pretest, serial_out_test, expect_match_reg_exps)

# ===========================================================================================