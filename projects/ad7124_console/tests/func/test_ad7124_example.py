import pytest
import serial
from time import sleep
import re
import enum

# could make these fixtures if there is a need to share them more widely
short_time = 0.1
long_time = 0.2

#===========================================================================================

def test_reset_action(serial_port, target_reset):
    """Test the device reset action"""

    expect_match_string = b'Reset Complete'
    expect_match_string_found = False

    # clear any pending input
    serial_port.reset_input_buffer()

    # Send the Reset Menu command, wait for input to arrive
    serial_port.write(b'A')
    sleep(short_time)

    # Read a large number of lines in case VT100 clear screen not used
    # port must have been opened with a timeout for this to work
    lines = serial_port.readlines(400)

    for line in lines:
        if expect_match_string in line:
            expect_match_string_found = True
            print()
            print(line.decode())
            break

    assert expect_match_string_found

    # Press any key to continue
    serial_port.write(b'!')
    sleep(short_time)

#===========================================================================================

def test_read_id(serial_port, target_reset):
    """Read the Device ID register and compares vs expected value"""

    expect_id_string_a = b'0x14'
    expect_id_string_b = b'0x16'
    expect_id_string_found = False

    # clear any pending input
    serial_port.reset_input_buffer()

    # Send the Read ID Register command, wait for input to arrive
    serial_port.write(b'D')
    # Press any key to continue
    serial_port.write(b'!')

    # Read a lines to see what ID is returned
    lines = serial_port.readlines(50)

    for line in lines:
        # Datasheet specified value of ID register is 0x14/0x16
        if ((expect_id_string_a in line) or (expect_id_string_b in line)):
            expect_id_string_found = True
            print()
            print(line.decode())
            break

    assert expect_id_string_found

#===========================================================================================

def test_sample_config_a_single(serial_port, target_reset):
    """performs a single conversion with config a and checks sample value"""

    # Send the Sample Menu command, wait for input to arrive
    serial_port.write(b'F')
    sleep(short_time)

    # clear any pending input
    serial_port.reset_input_buffer()
    
    # Send the Single Conversion Menu command, wait for input to arrive
    serial_port.write(b'S')
    sleep(long_time)

    # Read lines to gather all the lines on console
    lines = serial_port.readlines(400)

    # Press any key to continue
    serial_port.write(b'!')
    # Press escape key to return to main menu
    serial_port.write(b'\x1b')

    # check for the conversion completion string
    single_conversion_string_found = False
    start_line = 0
    for idx, line in list(enumerate(lines)):
        if b'Single Conversion completed...' in line:
            single_conversion_string_found = True
            start_line = idx
            break

    assert (single_conversion_string_found)

    # check for the table header row
    table_header_row_found = False
    regex = r"Ch\s*Value\s*Count\s*Voltage"
    for idx, line in list(enumerate(lines))[start_line:]:
        # need to decode bytes objects to strings for regex to work
        if re.search(regex, line.decode()):
            table_header_row_found = True
            start_line = idx + 1
            break

    assert (table_header_row_found)

    # check the table row is present and contains sensible numbers
    # Requirement is that CH0 be on AIN0/AIN1 on the eval board
    # and the noise test jumper LK5 be fitted
    data_row_found = False

    # This regex parses this string, and returns two groups
    #    0       8388463         1               -0.000043
    # first is the count, e.g. 8388462, and second is the floating
    # point number value
    regex = r"\d*\s*(\d*)\s*1\s*([+-]?[0-9]*[.]?[0-9]+)"
    line = lines[start_line].decode()
    # need to decode bytes objects to strings for regex to work
    m = re.search(regex, line)
    if m:
        print()
        print("Sample Counts = ", m.group(1))
        print("Sample Voltage = ", m.group(2))
        # allow 300 counts above/below zero count mid-scale
        assert (8388308 <= int(m.group(1)) <= 8388908)
        # Allow 100uV above/below 0V  mid scale
        assert (-0.000100 <= float(m.group(2)) <= 0.000100)
    else:
        assert data_row_found

#===========================================================================================

def test_sample_config_a_stream(serial_port, target_reset):
    """performs a stream conversion with config a and checks sample values"""

    # Send the Sample Menu command, wait for input to arrive
    serial_port.write(b'F')
    sleep(short_time)
    # clear any pending input
    serial_port.reset_input_buffer()
    # Send the Continuous Conversion - Stream Menu command, wait for input to arrive
    serial_port.write(b'C')
    sleep(long_time)

    # Press escape key to stop sampling, wait for inout to arrive
    serial_port.write(b'\x1b')
    sleep(long_time)

    # Read lines to gather all the lines on console
    lines = serial_port.readlines(250)

    # check for the channel header string, just one channel '0'
    m = re.search(r"[0-9]*", lines[0].decode())

    assert m.group(0) == "0"

    # check first 10 rows of data contain sensible voltage values
    # Requirement is that CH0 be on AIN0/AIN1 on the eval board
    # and the noise test jumper LK5 be fitted

    # This regex parses this string, and returns two groups
    #    0       8388463         1               -0.000043
    # first is the count, e.g. 8388462, and second is the floating
    # point number value
    regex = r"[+-]?[0-9]*[.]?[0-9]+"

    print()
    for line in lines[1:11]:
        # need to decode bytes objects to strings for regex to work
        m = re.search(regex, line.decode())
        if m:
            print("Sample Voltage = ", m.group(0))
            # Allow 100uV above/below 0V  mid scale
            assert (-0.000100 <= float(m.group(0)) <= 0.000100)

    # Press any key to continue
    serial_port.write(b'!')
    # Press escape key to return to main menu
    serial_port.write(b'\x1b')

#===========================================================================================

def test_enable_channels(serial_port, target_reset):
    """Enables the channels"""

    # clear any pending input
    serial_port.reset_input_buffer()

    # Enter into channel enable/disable menu
    serial_port.write(b'H')
    sleep(short_time)

    # Enter into the channel enable submenu
    serial_port.write(b'e')
    sleep(short_time)

    enable_disable_channels(serial_port, "ENABLE")

    # Return to main menu
    serial_port.write(b'!')
    sleep(short_time)


def test_disable_channels(serial_port, target_reset):
    """Disables the channels"""

    # clear any pending input
    serial_port.reset_input_buffer()

    # Enter into channel enable/disable menu
    serial_port.write(b'H')
    sleep(short_time)

    # Enter into the channel disable submenu
    serial_port.write(b'd')
    sleep(short_time)

    enable_disable_channels(serial_port, "DISABLE")

    # Return to main menu
    serial_port.write(b'!')
    sleep(short_time)


def enable_disable_channels(serial_port, action):
    """Enables/Disables the channels and Verify them by reading the device setup"""

    expected_channel_selection_string = b'\tEnter Channel Value <0-15>: '
    channel_select_string_found = False

    # Enable the channels 2-14
    for chn in range(2,15):
        # Read the lines on console window
        lines = serial_port.readlines(500)

        for line in lines:
            if expected_channel_selection_string in line:
                channel_select_string_found = True

                 # Enable the current channel
                chn_str = str(chn) + '\r' + '\n'
                serial_port.write(chn_str.encode('utf-8'))
                sleep(short_time)

                 # Continue for next channel
                serial_port.write(b'y')
                sleep(short_time)
                break

        # Assert the failure if any
        assert (channel_select_string_found)

    # Enable channel 15 and write 'n' to come out of this menu
    serial_port.write(b'15\r\n')
    sleep(short_time)
    serial_port.write(b'n')
    sleep(short_time)

    # Press escape key to return to main menu
    serial_port.write(b'\x1b')
    sleep(short_time)

    serial_port.reset_input_buffer()

    # Read the setup to verify results
    serial_port.write(b'K')
    sleep(short_time)
    sleep(short_time)

    # Read the setup
    lines = serial_port.readlines(1000)

    # Find the setup
    regex = r"\s*Channel#\s*|\s*Enable\s*|\s*Setup\s*|\s*AINP\s*|\s*AINM"
    table_header_found = False
    start_line = 0
    for idx, line in list(enumerate(lines))[start_line:]:
        # need to decode bytes objects to strings for regex to work
        if re.search(regex, line.decode()):
            table_header_found = True
            start_line = idx + 1
            break

    assert table_header_found

    # Go to the 3rd row of device setup table (to start with channel 2)
    start_line = start_line + 3

    #    Chn       Enable         Setup       AINP      AINM
    chn_enable_expr = r"\d*\s*ENABLED\s*\d\s*\d\s*\d"
    chn_disable_expr = r"\d*\s*DISABLED\s*\d\s*\d\s*\d"

    chn = 2
    print()
    for line in lines[start_line:start_line+14]:
        if action == "ENABLE":
            print(line.decode())
            e = re.search(chn_enable_expr, line.decode())
            assert e, "Error in enabling channel!!"
            print('Channel', chn, 'is Enabled')
        elif action == "DISABLE":
            e = re.search(chn_disable_expr, line.decode())
            assert e, "Error in disabling channel!!"
            print('Channel', chn, 'is Disabled')

        chn = chn + 1

#===========================================================================================

# Channel and analog inputs mapping
channel_input_mapping = [
        # Channel  AINP  AINM
        [   2,      2,    17  ],
        [   3,      3,    17  ],
        [   4,      4,    17  ],
        [   5,      5,    17  ],
        [   6,      6,    17  ],
        [   7,      7,    17  ],
        [   8,      8,    17  ],
        [   9,      9,    17  ],
        [   10,     10,   17  ],
        [   11,     11,   17  ],
        [   12,     12,   17  ],
        [   13,     13,   17  ],
        [   14,     14,   15  ],
        [   15,     16,   17  ],
    ]

def test_connect_inputs_to_channel(serial_port, target_reset):
    """Connect analog inputs to channel and Verify them by reading the device setup"""

    # Clear any pending inputs
    serial_port.reset_input_buffer()
    sleep(short_time)

    for chn in range(2, 16):   # Channels 2 - 15

        # Enter into connect analog inputs to channel menu
        serial_port.write(b'I')
        sleep(short_time)

        # Select the channel
        chn_str = str(chn) + '\r' + '\n'
        serial_port.write(chn_str.encode('utf-8'))
        sleep(short_time)

        # Select the positive analog input (AINP)
        analog_input = str(channel_input_mapping[chn-2][1]) + '\r' + '\n'
        serial_port.write(analog_input.encode('utf-8'))
        sleep(short_time)

        # Select the negative analog input (AINM)
        analog_input = str(channel_input_mapping[chn-2][2]) + '\r' + '\n'
        serial_port.write(analog_input.encode('utf-8'))
        sleep(short_time)

        # Press any key to return to main menu
        serial_port.write(b'!')
        sleep(short_time)
        
        serial_port.reset_input_buffer()
        sleep(short_time)
        sleep(short_time)

    # Read the setup
    serial_port.reset_input_buffer()
    sleep(short_time)
    
    # Read the setup to verify results
    serial_port.write(b'K')
    sleep(short_time)

    lines = serial_port.readlines(1000)

    regex = r"\s*Channel#\s*|\s*Enable\s*|\s*Setup\s*|\s*AINP\s*|\s*AINM"
    table_header_found = False
    start_line = 0
    idx = 0
    print()
    for idx, line in list(enumerate(lines))[start_line:]:
        # need to decode bytes objects to strings for regex to work
        if re.search(regex, line.decode()):
            table_header_found = True
            start_line = idx + 1
            print(line)
            break

    assert table_header_found

    # Go to the 3rd row of device setup table (to start with channel 2)
    start_line = start_line + 3

    #    Chn       Enable         Setup       AINP      AINM
    chn_input_connection_expr = r"\s*(\d*)\s*([a-zA-Z]*)\s*(\d*)\s*(\d*)\s*(\d*)"

    # Verify that all channels are connected to respective inputs
    chn = 2
    for line in lines[start_line:start_line+14]:
        c = re.search(chn_input_connection_expr, line.decode())
        if c:
            assert (int(c.group(4)) == channel_input_mapping[chn-2][1]), "Error in input connection!!"
            assert (int(c.group(5)) == channel_input_mapping[chn-2][2]), "Error in input connection!!"
        else:
            assert c, "Error in input connection!!"

        print(line)
        chn = chn + 1

    # Return to main menu
    serial_port.write(b'!')
    sleep(short_time)

#===========================================================================================

class filter_type(enum.Enum): 
    SINC4 = 0
    SINC3 = 1
    FS_SINC4 = 2
    FS_SINC3 = 3

class ref_src(enum.Enum): 
    REFIN1 = 0
    REFIN2 = 1
    INTERNAL = 2
    AVDD = 3

filter_name = ['SINC4', 'SINC3', 'SINC3', 'FS SINC4', 'FS SINC3', 'SINC3', 'SINC4', 'SINC4']
ref_src_name = ['REFIN1', 'REFIN1', 'REFIN2', 'AVDD', 'INTERNAL', 'REFIN2', 'AVDD', 'REFIN1']
gain_values = [1, 1, 2, 16, 4, 128, 8, 32]

setup_config = [
    # Setup#    Filter Type       Data Rate    AIN_BUFP   AIN_BUFM    REF_BUFP   REF_BUFM    Polarity    Gain bits  REF SOURCE     Channel to Assign Setup
    [   0,    filter_type.SINC4,      120,         1,         1,         0,         0,          1,        1,     ref_src.REFIN1,         3               ],
    [   1,    filter_type.SINC3,       60,         1,         1,         0,         0,          1,        1,     ref_src.REFIN1,         7               ],
    [   2,    filter_type.SINC3,       60,         0,         0,         1,         1,          1,        2,     ref_src.REFIN2,        11               ],
    [   3,    filter_type.FS_SINC4,    31,         0,         0,         1,         1,          1,        4,     ref_src.AVDD,           9               ],
    [   4,    filter_type.FS_SINC3,    30,         1,         1,         1,         1,          1,        8,     ref_src.INTERNAL,       6               ],
    [   5,    filter_type.SINC3,       40,         1,         1,         1,         1,          1,       16,     ref_src.REFIN2,        13               ],
    [   6,    filter_type.SINC4,      120,         0,         0,         0,         0,          0,      128,     ref_src.AVDD,           4               ],
    [   7,    filter_type.SINC4,      120,         1,         1,         0,         0,          0,       64,     ref_src.REFIN1,        15               ]
]

def test_config_and_assign_setup(serial_port, target_reset):
    """Configure and assign setup to channel and Verify it by reading the device setup"""

    serial_port.reset_input_buffer()

    for setup in range(0, 8):   # Setup 0 - 7

        # Enter into connect analog inputs to channel menu
        serial_port.write(b'J')
        sleep(short_time)

        # Select the setup
        setup_str = str(setup) + '\r' + '\n'
        serial_port.write(setup_str.encode('utf-8'))
        sleep(short_time)

        # Select the filter type
        filter_type = str(setup_config[setup][1].value) + '\r' + '\n'
        serial_port.write(filter_type.encode('utf-8'))
        sleep(short_time)

        # Select the data rate
        data_rate = str(setup_config[setup][2]) + '\r' + '\n'
        serial_port.write(data_rate.encode('utf-8'))
        sleep(short_time)

        # Select the gain
        gain_inputs = str(setup_config[setup][8]) + '\r' + '\n'
        serial_port.write(gain_inputs.encode('utf-8'))
        sleep(short_time)

        # Select the polarity
        polarity = str(setup_config[setup][7]) + '\r' + '\n'
        serial_port.write(polarity.encode('utf-8'))
        sleep(short_time)

        # Select the analog buffers
        analog_buffers = str(setup_config[setup][3]) + '\r' + '\n'
        serial_port.write(analog_buffers.encode('utf-8'))
        sleep(short_time)

        # Select the reference buffers
        ref_buffers = str(setup_config[setup][5]) + '\r' + '\n'
        serial_port.write(ref_buffers.encode('utf-8'))
        sleep(short_time)

        # Select the reference source
        ref_src = str(setup_config[setup][9].value) + '\r' + '\n'
        serial_port.write(ref_src.encode('utf-8'))
        sleep(short_time)

        # Assign setup to channel
        serial_port.write(b'y')
        sleep(short_time)

        # Select the channel
        chn_str = str(setup_config[setup][10]) + '\r' + '\n'
        serial_port.write(chn_str.encode('utf-8'))
        sleep(short_time)

        # Press any key to return to main menu
        serial_port.write(b'!')
        sleep(short_time)
        
        serial_port.reset_input_buffer()
        sleep(short_time)
        sleep(short_time)

    # Read the setup to verify results
    serial_port.reset_input_buffer()
    sleep(short_time)
        
    serial_port.write(b'K')
    sleep(short_time)

    lines = serial_port.readlines(2000)

    regex = r"\s*Setup#\s*|\s*Filter Type\s*|\s*Data Rate\s*|\s*AIN_BUFP\s*|\s*AIN_BUFM\s*|\s*REF_BUFP\s*|\s*REF_BUFM\s*|\s*Polarity\s*|\s*Gain\s*|\s*REF SOURCE"
    table_header_found = False
    start_line = 0
    idx = 0
    print()
    for idx, line in list(enumerate(lines))[start_line:]:
        # need to decode bytes objects to strings for regex to work
        if re.search(regex, line.decode()):
            table_header_found = True
            start_line = idx + 1
            print(line)
            break

    assert table_header_found

    # Go to the 1st row of device setup table (to start with setup 0)
    start_line = start_line + 1

    # The entire string to be searched is too long to search for regex module. So the string
    # is divided into 3 parts and regular expression is created for each string match detect.

    #    Setup    Filter Type    Data Rate       
    setup_config_expr = r"\s*(\d*)\s*([a-zA-Z]*\s*[a-zA-Z]*[0-9]*)\s*(\d*)"
    #    AIN_BUFP    AIN_BUFM    REF_BUFP    REF_BUFM 
    setup_config_expr1 = r"\s*([A-Z]*)\s*([A-Z]*)\s*([A-Z]*)\s*([A-Z]*)"
    #    Polarity    Gain    REF SOURCE
    setup_config_expr2 = r"\s*([A-Z]*)\s*([0-9]*)\s*([a-zA-Z]*[0-9]*)"

    # Verify that all channels are connected to respective inputs
    setup = 0
    for line in lines[start_line:start_line+8]:

        # Search for the first part of received string.
        c = re.search(setup_config_expr, line.decode())
        if c:
            assert (int(c.group(1)) == setup_config[setup][0]), "Error in setup " + str(setup) + " configuration!!"

            assert (str(c.group(2)) == filter_name[setup]), "Error in setup " + str(setup) + " configuration!!"

            assert (int(c.group(3)) == setup_config[setup][2]), "Error in setup " + str(setup) + " configuration!!"
        else:
            assert c, "Error in setup " + str(setup) + " configuration!!"


        # Search for the second part of received string. 
        # *Note: span() method provides the length of matched string. However, each part of received string
        # is seperated by 3 spaces, the span value must be added with 3 to begin searching the next string.
        d = re.search(setup_config_expr1, line[c.span()[1] + 3:].decode())
        if d:
            if int(setup_config[setup][3]) == 1:
                assert (str(d.group(1)) == 'ENABLED'), "Error in setup " + str(setup) + " configuration!!"
            elif int(setup_config[setup][3]) == 0:
                assert (str(d.group(1)) == 'DISABLED'), "Error in setup " + str(setup) + " configuration!!"
            
            if int(setup_config[setup][5]) == 1:
                assert (str(d.group(3)) == 'ENABLED'), "Error in setup " + str(setup) + " configuration!!"
            elif int(setup_config[setup][5]) == 0:
                assert (str(d.group(3)) == 'DISABLED'), "Error in setup " + str(setup) + " configuration!!"
        else:
            assert d, "Error in setup " + str(setup) + " configuration!!"


        # Search for the third part of received string.
        # *Note: span() method provides the length of matched string. However, each part of received string
        # is seperated by 3 spaces, the span value must be added with 3 to begin searching the next string.
        e = re.search(setup_config_expr2, line[c.span()[1] + d.span()[1] + 3:].decode())    
        if e:
            if (setup_config[setup][7] == 1):
                assert (str(e.group(1)) == 'BIPOLAR'), "Error in setup " + str(setup) + " configuration!!"
            elif (setup_config[setup][7] == 0):
                assert (str(e.group(1)) == 'UNIPOLAR'), "Error in setup " + str(setup) + " configuration!!"

            assert (int(e.group(2)) == setup_config[setup][8]), "Error in setup " + str(setup) + " configuration!!"

            assert (str(e.group(3)) == ref_src_name[setup]), "Error in setup " + str(setup) + " configuration!!"
        else:
            assert e, "Error in setup " + str(setup) + " configuration!!"

        # Print the received line (setup information)
        print(line)

        setup = setup + 1

    # Return to main menu
    serial_port.write(b'!')
    sleep(short_time)

#===========================================================================================

power_mode_select_inputs = [
    # Power Mode Menu, Mode to be selected, Display setup menu, Display string
    ['G', 'L', 'K', 'Power Mode: Low Power'],
    ['G', 'M', 'K', 'Power Mode: Medium Power'],
    ['G', 'F', 'K', 'Power Mode: Full Power'],
]

def test_power_mode_selection(serial_port, target_reset):
    """Change the power mode and verify it by reading device setup"""

    for power_mode in range (0, 3):
        # Clear any pending inputs
        serial_port.reset_input_buffer()
        sleep(short_time)

        # Enter into power mode change menu
        serial_port.write(power_mode_select_inputs[power_mode][0].encode('utf-8'))
        sleep(short_time)

        # Select low power mode
        serial_port.write(power_mode_select_inputs[power_mode][1].encode('utf-8'))
        sleep(short_time)

        # Return to main menu
        serial_port.write(b'!')
        sleep(short_time)

        # Press escape key to return to main menu
        serial_port.write(b'\x1b')
        sleep(short_time)

        serial_port.reset_input_buffer()
        sleep(short_time)
        
        # Read the setup to verify results
        serial_port.write(power_mode_select_inputs[power_mode][2].encode('utf-8'))
        sleep(short_time)

        matching_string_found = False
        lines = serial_port.readlines(1000)
        print()
        for line in lines:
            if power_mode_select_inputs[power_mode][3].encode('utf-8') in line:
                print(line)
                matching_string_found = True
                break

        assert matching_string_found

        # Return to main menu
        serial_port.write(b'!')
        sleep(short_time)

#===========================================================================================

def test_read_temperature(serial_port, target_reset):
    """Read the die temperature and verify if it's within specified limit"""
    serial_port.reset_input_buffer()

    # Query to read temperature
    serial_port.write(b'L')
    sleep(3)

    lines = serial_port.readlines(100)

    temperature = 0
    regex = r"Temperature: (\d*) Celcius"
    for line in lines:
        p = re.search(regex, line.decode())
        if p:
            temperature = p.group(1)
            assert (1 <= int(temperature) <= 50), "Error reading temperature!!"
            print('\nTemperature:', temperature)
            break

    # Return to main menu
    serial_port.write(b'!')
    sleep(short_time)

#===========================================================================================

def test_calibrate_adc(serial_port, target_reset):
    """Perform the adc calibration and verify the results"""

    serial_port.reset_input_buffer()
    sleep(short_time)

    # Enter into power mode change menu
    serial_port.write(b'M')
    sleep(long_time)
    sleep(long_time)
    sleep(long_time)

    lines = serial_port.readlines(5000)
    sleep(short_time)

    start_lines = 0
    for chn_cnt in range (0, 16):
        matching_string_found = False
        print()
        for line in list(enumerate(lines))[start_lines:]:
            expected_chn_str = "Calibrating Channel " + str(chn_cnt) + " =>"
            if expected_chn_str in line[1].decode():
                print(line[1])
                matching_string_found = True
                break

            start_lines = start_lines + 1

        assert matching_string_found

        matching_string_found = False
        print()
        for line in list(enumerate(lines))[start_lines:]:
            expected_cal_str = "Calibration Successful..."
            if expected_cal_str in line[1].decode():
                print(line[1])
                matching_string_found = True
                break

            start_lines = start_lines + 1
        
        assert matching_string_found

    # Return to main menu
    serial_port.write(b'!')
    sleep(short_time)

#===========================================================================================

def test_read_write_register(serial_port, target_reset):
    """Verify device read and write functionality"""

    # Clear any pending inputs
    serial_port.reset_input_buffer()
    sleep(short_time)

    # Enter into power mode change menu
    serial_port.write(b'N')
    sleep(short_time)
        
    # Enter into write menu
    serial_port.write(b'W')
    sleep(short_time)

    # Send the register address
    reg_add = "9\r\n"
    serial_port.write(reg_add.encode('utf-8'))
    sleep(short_time)

    # Send the register data (random 24-bit value)
    reg_data = "8230\r\n"
    serial_port.write(reg_data.encode('utf-8'))
    sleep(short_time)

    # Return to read/write menu
    serial_port.write(b'!')
    sleep(short_time)

    # Clear any pending inputs
    serial_port.reset_input_buffer()
    sleep(short_time)

    # Enter into write menu
    serial_port.write(b'R')
    sleep(short_time)

    # Send the register address
    reg_add = "9\r\n"
    serial_port.write(reg_add.encode('utf-8'))
    sleep(short_time)

    lines = serial_port.readlines(1000)
    print()
    matching_string_found = False
    for line in lines:
        if b"Read Value: 0x8230" in line:
            print(line)
            matching_string_found = True
            break

    assert matching_string_found

    # Return to main menu
    serial_port.write(b'!')
    sleep(short_time)

#===========================================================================================
