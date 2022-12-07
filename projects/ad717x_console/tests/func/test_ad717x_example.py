import pytest
import serial
from time import sleep
import re
import enum

# could make these fixtures if there is a need to share them more widely
short_time = 0.1
long_time = 0.2

# Note1: This test is performed for the AD4111 device of AD411x/AD711x family only.
#        To test additional devices, few of the test cases might need modifications
#        based on device functionality, which is not in the scope of this test script


# Define the menu command dictionary with key:value pair
# Key: test menu name, Value: command to trigger menu
menu_command = {
    'test_menu_header_and_footer':' ',
    'test_read_id':'A',
    'sample_chn_main_menu':'C',
    'test_sample_chn_single_conv':'S',
    'test_sample_chn_cont_conv':'C',
    'chn_enable_disable_main_menu':'D',
    'test_enable_channels_menu':'E',
    'test_disable_channels_menu':'D',
    'test_connect_inputs_to_channel':'E',
    'test_config_and_assign_setup':'F',
    'display_setup_menu':'G',
    'test_read_temperature':'H',
    'test_calibrate_adc':'I',
    'test_open_wire_detection':'J',
    'test_read_write_register_menu':'K',
    'read_register_menu':'R',
    'write_register_menu':'W',
}


# Channel and analog inputs mapping
channel_input_mapping = [
        # Channel     INP+      INP-      Command
        [   0,      'VIN0',   'VINCOM',     'B',  ],
        [   1,      'VIN0',   'VIN1',       'A',  ],
        [   2,      'VIN1',   'VINCOM',     'D',  ],
        [   3,      'VIN1',   'VIN0',       'C',  ],
        [   4,      'VIN2',   'VINCOM',     'F',  ],
        [   5,      'VIN2',   'VIN3',       'E',  ],
        [   6,      'VIN4',   'VINCOM',     'J',  ],
        [   7,      'VIN5',   'VINCOM',     'L',  ],
        [   8,      'VIN5',   'VIN4',       'K',  ],
        [   9,      'VIN3',   'VIN2',       'G',  ],
        [   10,     'VIN6',   'VIN7',       'M',  ],
        [   11,     'VIN7',   'VINCOM',     'P',  ],
        [   12,     'IN3+',   'IN3-',       'Q',  ],
        [   13,     'IN1+',   'IN1-',       'S',  ],
        [   14,     'TEMP+',  'TEMP-',      '7',  ],
        [   15,     'REF+',   'REF-',       '8',  ],
    ]

pin_map_cmd_indx = 3
pin_map_pos_inp_indx = 1
pin_map_neg_inp_indx = 2


# Filter select command
sinc5_1_filter_cmd = 'A'
sinc3_filter_cmd = 'B'

# ODR select command
odr_31250_cmd = 'A'
odr_5208_cmd = 'D'
odr_20_01_cmd = 'M'
odr_3906_cmd = 'E'
odr_401_cmd = 'H'
odr_102_cmd = 'I'
odr_50_cmd = 'K'

# Reference source select command
external_ref_cmd = 'A'
internal_ref_cmd = 'B'
avdd_vss_ref_cmd = 'C'

# Post filter selection command
_27_sps_cmd = 'A'
_25_sps_cmd = 'B'
_20_sps_cmd = 'C'
_16_sps_cmd = 'D'

setup_config_cmd = [
    # Setup#    Filter Type       Post Filter    Data Rate      AIN_BUFP   AIN_BUFM    REF_BUFP   REF_BUFM   Polarity     REF SOURCE     Channel
    [   0,    sinc3_filter_cmd,   _27_sps_cmd,  odr_102_cmd,     'E',       'E',        'E',        'E',      'B',    external_ref_cmd,   3   ],
    [   1,    sinc5_1_filter_cmd, _27_sps_cmd,  odr_31250_cmd,   'D',       'D',        'E',        'E',      'B',    external_ref_cmd,   7   ],
    [   2,    sinc5_1_filter_cmd, _20_sps_cmd,  odr_5208_cmd,    'E',       'E',        'D',        'D',      'U',    internal_ref_cmd,   11  ],
    [   3,    sinc3_filter_cmd,   _27_sps_cmd,  odr_401_cmd,     'E',       'E',        'D',        'D',      'U',    avdd_vss_ref_cmd,   9   ],
    [   4,    sinc3_filter_cmd,   _27_sps_cmd,  odr_3906_cmd,    'D',       'D',        'D',        'D',      'U',    internal_ref_cmd,   6   ],
    [   5,    sinc5_1_filter_cmd, _16_sps_cmd,  odr_20_01_cmd,   'D',       'D',        'D',        'D',      'B',    external_ref_cmd,   13  ],
    [   6,    sinc3_filter_cmd,   _27_sps_cmd,  odr_31250_cmd,   'E',       'E',        'E',        'E',      'B',    internal_ref_cmd,   4   ],
    [   7,    sinc3_filter_cmd,   _27_sps_cmd,  odr_50_cmd,      'E',       'E',        'E',        'E',      'B',    avdd_vss_ref_cmd,   15  ]
]

# Expected parameter values in the above configured setup
filter_type = ['Sinc3', 'Sinc5+1', 'Sinc5+1', 'Sinc3', 'Sinc3', 'Sinc5+1', 'Sinc3', 'Sinc3']
post_filter_type = ['NA(Disable)', '27_SPS(Enable)', '20_SPS(Enable)', 'NA(Disable)', 'NA(Disable)', '16_SPS(Enable)', 'NA(Disable)', 'NA(Disable)']
ref_src_name = ['External', 'External', 'Internal', 'AVDD-AVSS', 'Internal', 'External', 'Internal', 'AVDD-AVSS']
odr_values = [102.00, 31250.00, 5208.00, 401.00, 3906.00, 20.01, 31250.00, 50.00]

# Indexes for setup_config_cmd
setup_index = 0
filter_type_indx = 1
post_filter_indx = 2
odr_indx = 3
ain_buf_indx = 4
ref_buf_indx = 6
polarity_indx = 8
ref_source_indx = 9
channel_index = 10


# Defines for the open wire detection functionality testing
input_type_cmds = [('Single Ended', 'S'), ('Differential Ended', 'D')]

single_ended_chn_inp_pair_cmds = [('Channel 0', 'Channel 15', 'VIN0,VINCOM', 'A'), ('Channel 3' ,'Channel 4', 'VIN2,VINCOM', 'C'), 
                                  ('Channel 7' ,'Channel 8', 'VIN4,VINCOM', 'E'), ('Channel 11' ,'Channel 12', 'VIN6,VINCOM', 'G')]

diff_ended_chn_inp_pair_cmds = [ ('Channel 5', 'Channel 6', 'VIN2,VIN3', 'B'), 
                                ('Channel 9', 'Channel 10', 'VIN4,VIN5', 'C'), ('Channel 13', 'Channel 14', 'VIN6,VIN7', 'D')]

chn_inp_pairs = [single_ended_chn_inp_pair_cmds, diff_ended_chn_inp_pair_cmds]



def test_menu_header_and_footer(serial_port, target_reset):
    """test the menu header and footer information"""

    expected_header_info_string = b"Device: AD4111"
    expected_header_info_string_found = False

    lines = serial_port.readlines(1000)
    for line in lines:
        if expected_header_info_string in line:
            print('\r\n' + line.decode())
            expected_header_info_string_found = True
            break

    assert expected_header_info_string_found


def test_read_id(serial_port, target_reset):
    """Read the Device ID register and compares vs expected value"""

    expected_id_string = b'0x30de'
    expect_id_string_found = False

    # clear any pending input
    clear_serial_input_buffer(serial_port)

    # Send the Read ID Register command, wait for input to arrive
    send_serial_command(serial_port, menu_command['test_read_id'])

    # Read a lines to see what ID is returned
    lines = serial_port.readlines(1000)

    for line in lines:
        if expected_id_string in line:
            print('\r\n' + line.decode())
            expect_id_string_found = True
            break

    assert expect_id_string_found 


def test_sample_chn_single_conv(serial_port, target_reset):
    """performs a single conversion and checks sample value"""

    prepare_setup_for_chn_sample(serial_port)

    # Send the Sample Menu command, wait for input to arrive
    send_serial_command(serial_port, menu_command['sample_chn_main_menu'])
    
    # Send the Single Conversion Menu command, wait for input to arrive
    send_serial_command(serial_port, menu_command['test_sample_chn_single_conv'])

    # Read lines to gather all the lines on console
    lines = serial_port.readlines(500)

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
    data_row_found = False

    # This regex parses this string, and returns two groups
    #    0       16777215         1               2.500000
    # first is the count, e.g. 16777215, and second is the floating
    # point number value
    regex = r"\d*\s*(1[0-9]*)\s*1*\s*([+-]?[0-9]*[.]?[0-9]+)"
    line = lines[start_line].decode()

    # need to decode bytes objects to strings for regex to work
    m = re.search(regex, line)
    if m:
        print()
        print("Sample Counts = ", m.group(1))
        print("Sample Voltage = ", m.group(2))
        # allow 67108 (0.1v) counts above/below expeced count scale
        assert (16710106 <= int(m.group(1)) <= 16844323)
        # Allow 0.1 above/below 2.5V scale
        assert (2.490000 <= float(m.group(2)) <= 2.510000)
    else:
        assert data_row_found


def test_sample_chn_cont_conv(serial_port, target_reset):
    """performs a stream conversion with config a and checks sample values"""

    prepare_setup_for_chn_sample(serial_port)

    # Send the Sample Menu command, wait for input to arrive
    send_serial_command(serial_port, menu_command['sample_chn_main_menu']) 
    
    # Send the Single Conversion Menu command, wait for input to arrive
    send_serial_command(serial_port, menu_command['test_sample_chn_cont_conv'])

    # Press escape key to stop sampling
    press_escape_key_to_continue(serial_port)
    sleep(long_time)

    # Read lines to gather all the lines on console
    lines = serial_port.readlines(500)

    start_line = 0
    for line in lines:
        start_line = start_line + 1
        if b'Please make a selection' in line:
            break

    # check for the channel header string, just one channel '0'
    m = re.search(r"[0-9]*", lines[start_line].decode())

    assert m.group(0) == "0"

    # check first 10 rows of data contain sensible voltage values

    # This regex parses this string, and returns two groups
    #    0       16777215         1               2.500000
    # first is the count, e.g. 16777215, and second is the floating
    # point number value
    regex = r"[+-]?[0-9]*[.]?[0-9]+"

    print()
    for line in lines[start_line+1:start_line+11]:
        # need to decode bytes objects to strings for regex to work
        m = re.search(regex, line.decode())
        if m:
            print("Sample Voltage = ", m.group(0))
            # Allow 0.1 above/below 2.5v scale
            assert (2.490000 <= float(m.group(0)) <= 2.510000)


def test_enable_channels_menu(serial_port, target_reset):
    """Enable the channels"""

    # clear any pending input
    clear_serial_input_buffer(serial_port)

    # Enter into channel enable/disable menu
    send_serial_command(serial_port, menu_command['chn_enable_disable_main_menu'])

    # Enter into the channel enable submenu
    send_serial_command(serial_port, menu_command['test_enable_channels_menu'])

    enable_disable_channels(serial_port, "ENABLE")


def test_disable_channels_menu(serial_port, target_reset):
    """Disable the channels"""

    # clear any pending input
    clear_serial_input_buffer(serial_port)

    # Enter into channel enable/disable menu
    send_serial_command(serial_port, menu_command['chn_enable_disable_main_menu'])

    # Enter into the channel enable submenu
    send_serial_command(serial_port, menu_command['test_disable_channels_menu'])

    enable_disable_channels(serial_port, "DISABLE")


def enable_disable_channels(serial_port, action):
    """Enables/Disables the channels and Verify them by reading the device setup"""

    expected_channel_selection_string = b'\tEnter Channel Value <0-15>: '
    channel_select_string_found = False

    # Enable the channels 0-14
    for chn in range(0,15):
        # Read the lines on console window
        lines = serial_port.readlines(5000)

        for line in lines:
            if expected_channel_selection_string in line:
                channel_select_string_found = True

                 # Enable the current channel
                chn_str = str(chn) + '\n'
                send_serial_command(serial_port, chn_str)

                 # Continue for next channel
                send_serial_command(serial_port, 'y')
                break

        # Assert the failure if any
        assert (channel_select_string_found)

    # Enable channel 15 and write 'n' to come out of this menu
    send_serial_command(serial_port, '15\n')
    send_serial_command(serial_port, 'n')

    # Press escape key to return to main menu
    press_escape_key_to_continue(serial_port)

    clear_serial_input_buffer(serial_port)

    # Read the setup to verify results
    send_serial_command(serial_port, menu_command['display_setup_menu'])

    # Read the setup
    lines = serial_port.readlines(1000)

    # Find the setup
    regex = r"\s*Channel#\s*|\s*Status\s*|\s*Setup\s*|\s*INP0\s*|\s*INP1"
    table_header_found = False
    start_line = 0
    for idx, line in list(enumerate(lines))[start_line:]:
        # need to decode bytes objects to strings for regex to work
        if re.search(regex, line.decode()):
            table_header_found = True
            start_line = idx + 1
            break

    assert table_header_found

    #    Chn       Enable         Setup       INP0      INP1
    chn_enable_expr = r"\d*\s*Enable\s*"
    chn_disable_expr = r"\d*\s*Disable\s*"

    chn = 0
    start_line = start_line + 1
    print()
    for line in lines[start_line:start_line+16]:
        print(line)
        if action == "ENABLE":
            e = re.search(chn_enable_expr, line.decode())
            assert e, "Error in enabling channel!!"
        elif action == "DISABLE":
            e = re.search(chn_disable_expr, line.decode())
            assert e, "Error in disabling channel!!"

        chn = chn + 1


def test_connect_inputs_to_channel(serial_port, target_reset):
    """Connect analog inputs to channel and Verify them by reading the device setup"""

    # Clear any pending inputs
    clear_serial_input_buffer(serial_port)

    # Enter into connect analog inputs to channel menu
    send_serial_command(serial_port, menu_command['test_connect_inputs_to_channel'])

    for chn in range(0, 16):   # Channels 0 - 15

        # Select the analog input pair
        analog_input = channel_input_mapping[chn][pin_map_cmd_indx]
        send_serial_command(serial_port, analog_input)

        # Select the channel
        chn_str = str(chn) + '\n'
        send_serial_command(serial_port, chn_str)

        # Press any key to return to main menu
        press_any_key_to_continue(serial_port)
        
        clear_serial_input_buffer(serial_port)
        sleep(short_time)
    
    press_escape_key_to_continue(serial_port)

    clear_serial_input_buffer(serial_port)

    # Read the setup to verify results
    send_serial_command(serial_port, menu_command['display_setup_menu'])

    lines = serial_port.readlines(1000)

    regex = r"\s*Channel#\s*|\s*Status*|\s*Setup\s*|\s*INP0\s*|\s*INP1\s*"
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

    # Go to the 1st row of device setup table (to start with channel 0)
    start_line = start_line + 1

    #    Chn       Enable         Setup       INP0      INP1
    chn_input_connection_expr = r"\s*(\d*)\s*([a-zA-Z]*)\s*(\d*)\s*([a-zA-Z0-9+-]*)\s*([a-zA-Z0-9+-]*)"

    # Verify that all channels are connected to respective inputs
    chn = 0
    for line in lines[start_line:start_line+16]:
        c = re.search(chn_input_connection_expr, line.decode())
        if c:
            assert (str(c.group(4)) == channel_input_mapping[chn][pin_map_pos_inp_indx]), "Error in +ve input connection!!"
            assert (str(c.group(5)) == channel_input_mapping[chn][pin_map_neg_inp_indx]), "Error in -ve input connection!!"
        else:
            assert c, "Error in input connection!!"

        print(line)
        chn = chn + 1


def test_config_and_assign_setup(serial_port, target_reset):
    """Configure and assign setup to channel and Verify it by reading the device setup"""

    clear_serial_input_buffer(serial_port)

    for setup in range(0, 8):   # Setup 0 - 7

        # Enter into setup config menu
        send_serial_command(serial_port, menu_command['test_config_and_assign_setup'])

        # Select the setup
        setup_str = str(setup) + '\n'
        send_serial_command(serial_port, setup_str)

        # Select the filter type
        send_serial_command(serial_port, setup_config_cmd[setup][filter_type_indx])

        # Select the post filter enable/disable status and its type
        if setup_config_cmd[setup][filter_type_indx] == sinc5_1_filter_cmd:
            send_serial_command(serial_port, 'E')
            send_serial_command(serial_port, setup_config_cmd[setup][post_filter_indx])

        # Select the data rate
        send_serial_command(serial_port, setup_config_cmd[setup][odr_indx])

        # Select the polarity
        send_serial_command(serial_port, setup_config_cmd[setup][polarity_indx])

        # Select the reference source
        send_serial_command(serial_port, setup_config_cmd[setup][ref_source_indx])

        # Select the reference buffers
        send_serial_command(serial_port, setup_config_cmd[setup][ref_buf_indx])

        # Select the analog buffers
        send_serial_command(serial_port, setup_config_cmd[setup][ain_buf_indx])

        # Assign setup to channel
        send_serial_command(serial_port, 'y')

        # Select the channel
        chn_str = str(setup_config_cmd[setup][channel_index]) + '\n'
        send_serial_command(serial_port, chn_str)

        # Press any key to select next channel
        press_any_key_to_continue(serial_port)
        
        # Exit to main menu without selecting another channel
        send_serial_command(serial_port, 'n')
        
        clear_serial_input_buffer(serial_port)

    clear_serial_input_buffer(serial_port)
    sleep(short_time)

    # Read the setup to verify results   
    send_serial_command(serial_port, menu_command['display_setup_menu'])

    lines = serial_port.readlines(5000)

    regex = r"\s*Setup#\s*|\s*Filter\s*|\s*Post Filter\s*|\s*Data Rate\s*|\s*INPBUF+\s*|\s*INPBUF-\s*|\s*REFBUF+\s*|\s*REFBUF-\s*|\s*Polarity\s*|\s*Ref Source"
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

    #    Setup    Filter  Post Filter    Data Rate       
    setup_config_expr = r"\s*(\d*)\s*([a-zA-Z0-9+]*)\s*([a-zA-Z0-9_()]*)\s*([0-9.]*)"
    #    INPBUF+    INPBUF-    REFBUF+    REFBUF- 
    setup_config_expr1 = r"\s*([A-Za-z]*)\s*([A-Za-z]*)\s*([A-Za-z]*)\s*([A-Za-z]*)"
    #    Polarity   REF SOURCE
    setup_config_expr2 = r"\s*([A-Za-z]*)\s*([A-Za-z0-9-]*)"

    # Verify that all channels are connected to respective inputs
    setup = 0
    for line in lines[start_line:start_line+8]:
        # Print the received line (setup information)
        print(line)

        # Search for the first part of received string.
        c = re.search(setup_config_expr, line.decode())
        if c:
            assert (int(c.group(1)) == setup_config_cmd[setup][setup_index]), "Error in setup " + str(setup) + " configuration!!"

            assert (str(c.group(2)) == filter_type[setup]), "Error in setup " + str(setup) + " filter configuration!!"

            assert (str(c.group(3)) == post_filter_type[setup]), "Error in setup " + str(setup) + " post filter configuration!!"

            assert (float(c.group(4)) == odr_values[setup]), "Error in setup " + str(setup) + " ODR configuration!!"
        else:
            assert c, "Error in setup " + str(setup) + " configuration!!"


        # Search for the second part of received string. 
        # *Note: span() method provides the length of matched string. However, each part of received string
        # is seperated by 3 spaces, the span value must be added with 3 to begin searching the next string.
        d = re.search(setup_config_expr1, line[c.span()[1] + 3:].decode())
        if d:
            if setup_config_cmd[setup][ain_buf_indx] == 'E':
                assert (str(d.group(1)) == 'Enable'), "Error in setup " + str(setup) + " AINBUF+ configuration!!"
            elif setup_config_cmd[setup][ain_buf_indx] == 'D':
                assert (str(d.group(1)) == 'Disable'), "Error in setup " + str(setup) + " AINBUF- configuration!!"
            
            if setup_config_cmd[setup][ref_buf_indx] == 'E':
                assert (str(d.group(3)) == 'Enable'), "Error in setup " + str(setup) + " REFBUF+ configuration!!"
            elif setup_config_cmd[setup][ref_buf_indx] == 'D':
                assert (str(d.group(3)) == 'Disable'), "Error in setup " + str(setup) + " REFBUF- configuration!!"
        else:
            assert d, "Error in setup " + str(setup) + " configuration!!"


        # Search for the third part of received string.
        # *Note: span() method provides the length of matched string. However, each part of received string
        # is seperated by 3 spaces, the span value must be added with 3 to begin searching the next string.
        e = re.search(setup_config_expr2, line[c.span()[1] + d.span()[1] + 3:].decode())    
        if e:
            if setup_config_cmd[setup][polarity_indx] == 'B':
                assert (str(e.group(1)) == 'Bipolar'), "Error in setup " + str(setup) + " polarity configuration!!"
            elif setup_config_cmd[setup][polarity_indx] == 'U':
                assert (str(e.group(1)) == 'Unipolar'), "Error in setup " + str(setup) + " polarity configuration!!"

            assert (str(e.group(2)) == ref_src_name[setup]), "Error in setup " + str(setup) + " ref source configuration!!"
        else:
            assert e, "Error in setup " + str(setup) + " configuration!!"

        setup = setup + 1
      

def test_read_temperature(serial_port, target_reset):
    """Read the die temperature and verify if it's within specified limit"""
    clear_serial_input_buffer(serial_port)

    # Query to read temperature
    send_serial_command(serial_port, menu_command['test_read_temperature'])
    sleep(long_time)

    lines = serial_port.readlines(1000)

    temperature = 0
    regex = r"Temperature: ([0-9]*.[0-9]*]*) Celcius"
    for line in lines:
        p = re.search(regex, line.decode())
        if p:
            temperature = p.group(1)
            assert (1.00 <= float(temperature) <= 50.00), "Error reading temperature!!"
            print('\r\nTemperature:', temperature)
            break


def test_calibrate_adc(serial_port, target_reset):
    """Perform the adc calibration and verify the results"""

    clear_serial_input_buffer(serial_port)

    # Enter into adc calibrate menu
    send_serial_command(serial_port, menu_command['test_calibrate_adc'])
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


def test_open_wire_detection(serial_port, target_reset):
    """ Test the open wire detection menu functionality (applicable only for AD4111 device) """

    # Expected string for floating/NC analog inputs on the target
    expected_result_string = b'Open Wire Detected on Selected Input Pair!!'

    # Select the single/differential ended input type for the open wire detection
    indx = 0
    print()
    for input_type, input_type_cmd in input_type_cmds:
        print(input_type + ": ")

        # Select the defined channel and input pairs for the open wire detection
        for chn1, chn2, input_pair, chn_inp_pair_cmd in chn_inp_pairs[indx]:

            # Enter into open wire detection menu
            send_serial_command(serial_port, menu_command['test_open_wire_detection'])

            # Send command to select input type
            send_serial_command(serial_port, input_type_cmd)

            # Send command to select channel type pair
            send_serial_command(serial_port, chn_inp_pair_cmd)

            # Send command to select input type pair
            send_serial_command(serial_port, chn_inp_pair_cmd)

            lines = serial_port.readlines(5000)

            expected_string_found = False
            for line in lines:
                if chn1 in line.decode('utf-8'):
                    print(line)

                if chn2 in line.decode('utf-8'):
                    print(line)
                
                if expected_result_string in line:
                    print(line)
                    expected_string_found = True
                    break

            assert expected_string_found

            # Continue for next channel-input pair
            press_any_key_to_continue(serial_port)
            clear_serial_input_buffer(serial_port)

        # Continue for next input type
        indx = indx + 1


def test_read_write_register_menu(serial_port, target_reset):
    """Verify device read and write functionality"""

    # Clear any pending inputs
    clear_serial_input_buffer(serial_port)

    # Enter into read menu
    send_serial_command(serial_port, menu_command['test_read_write_register_menu'])
        
    # Enter into write menu
    send_serial_command(serial_port, menu_command['write_register_menu'])

    # Send the register address
    reg_add = "10\n"
    send_serial_command(serial_port, reg_add)

    # Send the register data (random 24-bit value)
    reg_data = "8006\n"
    send_serial_command(serial_port, reg_data)

    # Return to read/write menu
    press_any_key_to_continue(serial_port)

    # Clear any pending inputs
    clear_serial_input_buffer(serial_port)

    # Enter into read menu
    send_serial_command(serial_port, menu_command['read_register_menu'])

    # Send the register address
    reg_add = "10\n"
    send_serial_command(serial_port, reg_add)

    lines = serial_port.readlines(1000)
    print()
    matching_string_found = False
    for line in lines:
        if b"Read Value: 0x8006" in line:
            print(line)
            matching_string_found = True
            break

    assert matching_string_found


#"""" Functions not part of the test script (without prefix test_) """"

def prepare_setup_for_chn_sample(serial_port):

    # Enable channel 0
    send_serial_command(serial_port, menu_command['chn_enable_disable_main_menu'])
    send_serial_command(serial_port, menu_command['test_enable_channels_menu'])
    send_serial_command(serial_port, '0\n')
    send_serial_command(serial_port, 'n')
    press_escape_key_to_continue(serial_port)
    
    clear_serial_input_buffer(serial_port)

    # Connect REF+ and REF- analog input pair to channel 0
    send_serial_command(serial_port, menu_command['test_connect_inputs_to_channel'])
    send_serial_command(serial_port, channel_input_mapping[15][pin_map_cmd_indx])
    send_serial_command(serial_port, '0\n')
    press_any_key_to_continue(serial_port)
    press_escape_key_to_continue(serial_port)
    
    clear_serial_input_buffer(serial_port)

    # Set polarity as Bipolar for setup0 (assigned to channel 0)
    send_serial_command(serial_port, menu_command['test_config_and_assign_setup'])
    send_serial_command(serial_port, '0\n')
    press_escape_key_to_continue(serial_port)   # skip filter selection
    press_escape_key_to_continue(serial_port)   # skip post filter selection
    press_escape_key_to_continue(serial_port)   # skip odr selection
    send_serial_command(serial_port, 'B')       # Set polarity s Bipolar
    press_escape_key_to_continue(serial_port)   # skip reference source selection
    press_escape_key_to_continue(serial_port)   # skip reference buffer selection
    press_escape_key_to_continue(serial_port)   # skip input buffer selection
    send_serial_command(serial_port, 'n')       # skip channel assignment

    clear_serial_input_buffer(serial_port)


def press_any_key_to_continue(serial_port):
    # Press any key to continue menu selections
    send_serial_command(serial_port, '!')


def press_escape_key_to_continue(serial_port):
    # Press escape key to continue menu selections
    send_serial_command(serial_port, '\x1b')


def send_serial_command(serial_port, command_key):
    # Send command key in byte format over the serial port
    serial_port.write(command_key.encode('utf-8'))
    sleep(short_time)
    sleep(short_time)


def clear_serial_input_buffer(serial_port):
    # Reset/Clear the serial input buffer
    serial_port.reset_input_buffer()
    sleep(short_time)
    sleep(short_time)