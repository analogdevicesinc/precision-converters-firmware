import pytest
import serial
from time import sleep
import re

# could make these fixtures if there is a need to share them more widely
short_time = 0.2
long_time = 1

# Note:  This test only supports the AD5686R nanodac+ device.
#        To test different nandodac+ devices, some test cases will need modifications
# 

number_of_dac_channels = 4      # This is device specific

# Define the DAC channel menu selection list
dac_channel_selection_list = [
    ['A', 'DAC Channel 0 is selected', False],  # Channel 0
    ['B', 'DAC Channel 1 is selected', False],  # Channel 1
    ['C', 'DAC Channel 2 is selected', False],  # Channel 2
    ['D', 'DAC Channel 3 is selected', False],  # Channel 3
]
# Define the indexes for items in list 'dac_channel_selection_list'
channel_select_cmd_indx = 0
channel_select_str_indx = 1
channel_select_flag_indx = 2


# DAC channel data/code list (hold random values)
dac_channel_data_list = [
    ['DAC Channel 0 Data: 32768', 32768],  # Channel 0
    ['DAC Channel 1 Data: 65346', 65346],  # Channel 1
    ['DAC Channel 2 Data: 12348', 12348],  # Channel 2
    ['DAC Channel 3 Data: 20001', 20001],  # Channel 3
]
# Define the indexes for items in list 'dac_channel_data_list'
channel_data_str_indx = 0
channel_data_val_indx = 1


# LDAC masks list
dac_channel_ldac_mask_list = [
    ['E', 'LDAC Mask for Channel 0: 1'],  # Channel 0
    ['D', 'LDAC Mask for Channel 1: 0'],  # Channel 1
    ['E', 'LDAC Mask for Channel 2: 1'],  # Channel 2
    ['D', 'LDAC Mask for Channel 3: 0'],  # Channel 3
]
# Define the indexes for items in list 'dac_channel_ldac_mask_list'
ldac_mask_cmd_indx = 0
ldac_mask_str_indx = 1


# LDAC pin state list
dac_channel_ldac_pin_list = [
    ['L', 'LDAC pin set to 0'],  # Channel 0
    ['L', 'LDAC pin set to 0'],  # Channel 1
    ['H', 'LDAC pin set to 1'],  # Channel 2
    ['H', 'LDAC pin set to 1'],  # Channel 3
]
# Define the indexes for items in list 'dac_channel_ldac_pin_list'
ldac_pin_cmd_indx = 0
ldac_pin_str_indx = 1


# Operating modes list
dac_channel_operating_mode_list = [
    ['A', 'Selected operating mode as Normal Power-Up'],    # Channel 0
    ['B', 'Selected operating mode as 1K to GND'],          # Channel 1
    ['C', 'Selected operating mode as 100K to GND'],        # Channel 2
    ['D', 'Selected operating mode as Three State'],        # Channel 3
]
# Define the indexes for items in list 'dac_channel_operating_mode_list'
operating_mode_cmd_indx = 0
operating_mode_str_indx = 1


# Vref source selection list
dac_channel_vref_source_list = [
    ['I', 'Vref Source: Internal', 'Vref Voltage: 2.500000'],  # Channel 0
    ['E', 'Vref Source: External', 'Vref Voltage: 3.500000'],  # Channel 1
    ['E', 'Vref Source: External', 'Vref Voltage: 3.500000'],  # Channel 2
    ['I', 'Vref Source: Internal', 'Vref Voltage: 2.500000'],  # Channel 3
]
# Define the indexes for items in list 'dac_channel_vref_source_list'
ext_vref_voltage = "3.5\r\n"
vref_source_cmd_indx = 0
vref_source_str_indx = 1
vref_vltg_str_indx = 2


# Gain selection list
dac_channel_gain_select_list = [
    ['1', 'Gain set to 1'],  # Channel 0
    ['2', 'Gain set to 2'],  # Channel 1
    ['2', 'Gain set to 2'],  # Channel 2
    ['1', 'Gain set to 1'],  # Channel 3
]
# Define the indexes for items in list 'dac_channel_gain_select_list'
gain_cmd_indx = 0
gain_str_indx = 1


# define the device reset data readback string list
device_reset_data_list = [
    'DAC Channel 0 Data: 0',    # Channel 0
    'DAC Channel 1 Data: 0',    # Channel 1
    'DAC Channel 2 Data: 0',    # Channel 2
    'DAC Channel 3 Data: 0',    # Channel 3
]


# Define the menu command dictionary with key:value pair
# Key: test menu name, Value: command to trigger menu
menu_command = {
    'test_menu_header_and_footer':' ',
    'test_menu_dac_channel_selection':'A',
    'test_menu_write_to_input_register':'B',
    'test_menu_update_dac_from_input':'C',
    'test_menu_update_dac_by_ldac_assert':'D',
    'test_menu_write_and_update_dac':'E',
    'test_menu_dac_readback':'F',
    'test_menu_ldac_masks':'G',
    'test_menu_select_ldac_pin_state':'H',
    'test_menu_dac_operating_modes':'I',
    'test_menu_vref_sources':'J',
    'test_menu_gain_selection':'K',
    'test_menu_assert_software_reset':'L',
    'test_menu_assert_hardware_reset':'M',
}


def test_menu_header_and_footer(serial_port, target_reset):
    """test the menu header and footer information"""

    expected_header_info_string = b"AD5686R (nanodac) | Vref:Internal (2.5V) | Gain:1"
    expected_header_info_string_found = False

    expected_footer_info_string = b"Active Channel: 0 | LDAC Pin: 1 | LDAC Mask: 0"
    expected_footer_info_string_found = False

    lines = serial_port.readlines(5000)
    print()
    for line in lines:
        if expected_header_info_string in line:
            print('Menu Header: ' + expected_header_info_string.decode())
            expected_header_info_string_found = True
            break

    assert expected_header_info_string_found, expected_header_info_string

    for line in lines:
        if expected_footer_info_string in line:
            print('Menu Footer: ' + expected_footer_info_string.decode())
            expected_footer_info_string_found = True
            break
            
    assert expected_footer_info_string_found, expected_footer_info_string


def test_menu_dac_channel_selection(serial_port, target_reset):
    """tests the dac channel selection menu"""

    # Clear the serial input buffers
    clear_serial_input_buffer(serial_port)

    # Write a command to display DAC channel selection menu
    send_serial_command(serial_port, menu_command['test_menu_dac_channel_selection'])
    
    print('\nDAC Channel Selections:')
    for dac_channel in range (0, number_of_dac_channels):
        # Write a command to select the DAC channel
        send_serial_command(serial_port, dac_channel_selection_list[dac_channel][channel_select_cmd_indx])

        # Read the data from serial buffer
        lines = serial_port.readlines(5000)
        for line in lines:
            # Check for expected string match
            if dac_channel_selection_list[dac_channel][channel_select_str_indx] in line.decode():
                dac_channel_selection_list[dac_channel][channel_select_flag_indx] = True
                break

        press_any_key_to_continue(serial_port)
        clear_serial_input_buffer(serial_port)

    # Assert and print the results
    for dac_channel in range (0, number_of_dac_channels):
        assert (dac_channel_selection_list[dac_channel][channel_select_flag_indx] == True), \
                dac_channel_selection_list[dac_channel][channel_select_str_indx]
        print(dac_channel_selection_list[dac_channel][channel_select_str_indx])


def test_menu_write_to_input_register(serial_port, target_reset):
    """tests the menu that writes to DAC input register"""

    # Clear the serial input buffers
    clear_serial_input_buffer(serial_port)

    print('\nDAC Channel Data (Readback Results):')
    for dac_channel in range (0, number_of_dac_channels):
        # Call a function that selects the DAC channel menu
        select_dac_channel(serial_port, dac_channel)

        # Write a command to invoke input data register write menu
        send_serial_command(serial_port, menu_command['test_menu_write_to_input_register'])

        # Write a code/data value from the local code/data list
        data_str = str(dac_channel_data_list[dac_channel][channel_data_val_indx]) + '\r' + '\n'
        send_serial_command(serial_port, data_str)

        press_any_key_to_continue(serial_port)

        # Verify the string by envoking DAC readback menu
        send_serial_command(serial_port, menu_command['test_menu_dac_readback'])

        # Verify the result
        expected_data_readback_string = dac_channel_data_list[dac_channel][channel_data_str_indx]
        expected_string_match_found = False
        print()

        lines = serial_port.readlines(5000)
        for line in lines:
            if expected_data_readback_string in line.decode():
                expected_string_match_found = True
                break

        assert expected_string_match_found, expected_data_readback_string
        print(expected_data_readback_string)

        press_any_key_to_continue(serial_port)
        clear_serial_input_buffer(serial_port)


def test_menu_update_dac_from_input(serial_port, target_reset):
    """tests the menu that writes the DAC input register contents to DAC channel register"""

    # Clear the serial input buffers
    clear_serial_input_buffer(serial_port)

    print('\nDAC Channel Data (Readback Results):')
    for dac_channel in range (0, number_of_dac_channels):
        # Call a function that selects the DAC channel menu
        select_dac_channel(serial_port, dac_channel)

        # Write a command to invoke input data register write menu
        send_serial_command(serial_port, menu_command['test_menu_write_to_input_register'])

        # Write a code/data value from the local code/data list
        data_str = str(dac_channel_data_list[dac_channel][channel_data_val_indx]) + '\r' + '\n'
        send_serial_command(serial_port, data_str)

        press_any_key_to_continue(serial_port)

        # Write a command to invoke menu to update DAC with input data register contents
        send_serial_command(serial_port, menu_command['test_menu_update_dac_from_input'])

        expected_result_string = 'Updated DAC register with contents of input register...'
        expected_string_match_found = False
        print()

        lines = serial_port.readlines(5000)
        for line in lines:
            if expected_result_string in line.decode():
                expected_string_match_found = True
                break

        assert expected_string_match_found, expected_result_string
        print(expected_result_string)

        press_any_key_to_continue(serial_port)

        # Verify the string by invoking DAC readback menu
        send_serial_command(serial_port, menu_command['test_menu_dac_readback'])

        # Verify the result
        expected_data_readback_string = dac_channel_data_list[dac_channel][channel_data_str_indx]
        expected_string_match_found = False
        print()

        lines = serial_port.readlines(5000)
        for line in lines:
            if expected_data_readback_string in line.decode():
                expected_string_match_found = True
                break

        assert expected_string_match_found, expected_data_readback_string
        print(expected_data_readback_string)

        press_any_key_to_continue(serial_port)
        clear_serial_input_buffer(serial_port)


def test_menu_update_dac_by_ldac_assert(serial_port, target_reset):
    """tests the menu that writes the DAC input register contents to DAC channel register by LDAC assert"""

    # Clear the serial input buffers
    clear_serial_input_buffer(serial_port)

    print('\nDAC Channel Data (Readback Results):')
    for dac_channel in range (0, number_of_dac_channels):
        # Call a function that selects the DAC channel menu
        select_dac_channel(serial_port, dac_channel)

        # Write a command to invoke input data register write menu
        send_serial_command(serial_port, menu_command['test_menu_write_to_input_register'])

        # Write a code/data value from the local code/data list
        data_str = str(dac_channel_data_list[dac_channel][channel_data_val_indx]) + '\r' + '\n'
        send_serial_command(serial_port, data_str)

        press_any_key_to_continue(serial_port)

        # Write a command to invoke menu to update DAC with input data register contents by LDAC assert
        send_serial_command(serial_port, menu_command['test_menu_update_dac_by_ldac_assert'])

        expected_result_string = 'Updated DAC register with contents of input register...'
        expected_string_match_found = False
        print()

        lines = serial_port.readlines(5000)
        for line in lines:
            if expected_result_string in line.decode():
                expected_string_match_found = True
                break

        assert expected_string_match_found, expected_result_string
        print(expected_result_string)

        press_any_key_to_continue(serial_port)

        # Verify the string by invoking DAC readback menu
        send_serial_command(serial_port, menu_command['test_menu_dac_readback'])

        # Verify the result
        expected_data_readback_string = dac_channel_data_list[dac_channel][channel_data_str_indx]
        expected_string_match_found = False
        print()

        lines = serial_port.readlines(5000)
        for line in lines:
            if expected_data_readback_string in line.decode():
                expected_string_match_found = True
                break

        assert expected_string_match_found, expected_data_readback_string
        print(expected_data_readback_string)

        press_any_key_to_continue(serial_port)
        clear_serial_input_buffer(serial_port)


def test_menu_write_and_update_dac(serial_port, target_reset):
    """tests the menu that writes the DAC registers directly"""

    # Clear the serial input buffers
    clear_serial_input_buffer(serial_port)

    print('\nDAC Channel Data (Readback Results):')
    for dac_channel in range (0, number_of_dac_channels):
        # Call a function that selects the DAC channel menu
        select_dac_channel(serial_port, dac_channel)

        # Write a command to invoke DAC register write menu
        send_serial_command(serial_port, menu_command['test_menu_write_and_update_dac'])

        # Write a code/data value from the local code/data list
        data_str = str(dac_channel_data_list[dac_channel][channel_data_val_indx]) + '\r' + '\n'
        send_serial_command(serial_port, data_str)

        press_any_key_to_continue(serial_port)

        # Verify the string by invoking DAC readback menu
        send_serial_command(serial_port, menu_command['test_menu_dac_readback'])

        # Verify the result
        expected_data_readback_string = dac_channel_data_list[dac_channel][channel_data_str_indx]
        expected_string_match_found = False
        print()

        lines = serial_port.readlines(5000)
        for line in lines:
            if expected_data_readback_string in line.decode():
                expected_string_match_found = True
                break

        assert expected_string_match_found, expected_data_readback_string
        print(expected_data_readback_string)

        press_any_key_to_continue(serial_port)
        clear_serial_input_buffer(serial_port)


def test_menu_ldac_masks(serial_port, target_reset):
    """tests the LDAC mask selection menu"""

    # Clear the serial input buffers
    clear_serial_input_buffer(serial_port)

    print('\nLDAC Mask Selections (For respective channels 0 to n):')
    for dac_channel in range (0, number_of_dac_channels):
        # Call a function that selects the DAC channel menu
        select_dac_channel(serial_port, dac_channel)

        # Write a command to invoke LDAC mask select menu
        send_serial_command(serial_port, menu_command['test_menu_ldac_masks'])

        # Write a LDAC mask select command
        send_serial_command(serial_port, dac_channel_ldac_mask_list[dac_channel][ldac_mask_cmd_indx])

        # Verify the result
        expected_ldac_mask_select_string = dac_channel_ldac_mask_list[dac_channel][ldac_mask_str_indx]
        expected_string_match_found = False
        print()

        lines = serial_port.readlines(5000)
        for line in lines:
            if expected_ldac_mask_select_string in line.decode():
                expected_string_match_found = True
                break

        assert expected_string_match_found, expected_ldac_mask_select_string
        print(expected_ldac_mask_select_string)

        press_any_key_to_continue(serial_port)
        press_escape_key_to_continue(serial_port)

        clear_serial_input_buffer(serial_port)


def test_menu_select_ldac_pin_state(serial_port, target_reset):
    """tests the LDAC pin selection menu"""

    # Clear the serial input buffers
    clear_serial_input_buffer(serial_port)

    print('\nLDAC Pin Selections (For respective channels 0 to n):')
    for dac_channel in range (0, number_of_dac_channels):
        # Call a function that selects the DAC channel menu
        select_dac_channel(serial_port, dac_channel)

        # Write a command to invoke LDAC pin select menu
        send_serial_command(serial_port, menu_command['test_menu_select_ldac_pin_state'])

        # Write a LDAC pin select command
        send_serial_command(serial_port, dac_channel_ldac_pin_list[dac_channel][ldac_pin_cmd_indx])

        # Verify the result
        expected_ldac_pin_select_string = dac_channel_ldac_pin_list[dac_channel][ldac_pin_str_indx]
        expected_string_match_found = False
        print()

        lines = serial_port.readlines(5000)
        for line in lines:
            if expected_ldac_pin_select_string in line.decode():
                expected_string_match_found = True
                break

        assert expected_string_match_found, expected_ldac_pin_select_string
        print(expected_ldac_pin_select_string)

        press_any_key_to_continue(serial_port)
        press_escape_key_to_continue(serial_port)
        
        clear_serial_input_buffer(serial_port)


def test_menu_dac_operating_modes(serial_port, target_reset):
    """tests the DAC operating mode selection menu"""

    # Clear the serial input buffers
    clear_serial_input_buffer(serial_port)

    print('\nLDAC Operating Mode Selections (For respective channels 0 to n):')
    for dac_channel in range (0, number_of_dac_channels):
        # Call a function that selects the DAC channel menu
        select_dac_channel(serial_port, dac_channel)

        # Write a command to invoke operating mode select menu
        send_serial_command(serial_port, menu_command['test_menu_dac_operating_modes'])

        # Write a operaing mode select command
        send_serial_command(serial_port, dac_channel_operating_mode_list[dac_channel][operating_mode_cmd_indx])

        # Verify the result
        expected_operating_mode_select_string = dac_channel_operating_mode_list[dac_channel][operating_mode_str_indx]
        expected_string_match_found = False
        print()

        lines = serial_port.readlines(5000)
        for line in lines:
            if expected_operating_mode_select_string in line.decode():
                expected_string_match_found = True
                break

        assert expected_string_match_found, expected_operating_mode_select_string
        print(expected_operating_mode_select_string)

        press_any_key_to_continue(serial_port)
        press_escape_key_to_continue(serial_port)
        
        clear_serial_input_buffer(serial_port)


def test_menu_vref_sources(serial_port, target_reset):
    """tests the menu to select vref source and voltage"""

    # Clear the serial input buffers
    clear_serial_input_buffer(serial_port)

    print('\nVref Source Selections (For respective channels 0 to n):')
    for dac_channel in range (0, number_of_dac_channels):
        # Call a function that selects the DAC channel menu
        select_dac_channel(serial_port, dac_channel)

        # Write a command to invoke vref source selection menu
        send_serial_command(serial_port, menu_command['test_menu_vref_sources'])

        # # Write a vref source select command
        send_serial_command(serial_port, dac_channel_vref_source_list[dac_channel][vref_source_cmd_indx])

        if (dac_channel_vref_source_list[dac_channel][vref_source_cmd_indx] == 'E'):
            send_serial_command(serial_port, ext_vref_voltage)

        # Verify the result
        expected_vref_source_string = dac_channel_vref_source_list[dac_channel][vref_source_str_indx]
        expected_string_match_found = False
        print()

        lines = serial_port.readlines(5000)
        for line in lines:
            if expected_vref_source_string in line.decode():
                expected_string_match_found = True
                break

        assert expected_string_match_found, expected_vref_source_string
        print(expected_vref_source_string)

        expected_vref_voltage_string = dac_channel_vref_source_list[dac_channel][vref_vltg_str_indx]
        expected_string_match_found = False
        print()

        for line in lines:
            if expected_vref_voltage_string in line.decode():
                expected_string_match_found = True
                break

        assert expected_string_match_found, expected_vref_voltage_string
        print(expected_vref_voltage_string)

        press_any_key_to_continue(serial_port)
        clear_serial_input_buffer(serial_port)


def test_menu_gain_selection(serial_port, target_reset):
    """tests the DAC gain selection menu"""

    # Clear the serial input buffers
    clear_serial_input_buffer(serial_port)

    print('\nDAC Gain Selections (For respective channels 0 to n):')
    for dac_channel in range (0, number_of_dac_channels):
        # Call a function that selects the DAC channel menu
        select_dac_channel(serial_port, dac_channel)

        # Write a command to invoke gain select menu
        send_serial_command(serial_port, menu_command['test_menu_gain_selection'])

        # Write a gain select command
        send_serial_command(serial_port, dac_channel_gain_select_list[dac_channel][gain_cmd_indx])

        # Verify the result
        expected_gain_select_string = dac_channel_gain_select_list[dac_channel][gain_str_indx]
        expected_string_match_found = False
        print()

        lines = serial_port.readlines(5000)
        for line in lines:
            if expected_gain_select_string in line.decode():
                expected_string_match_found = True
                break

        assert expected_string_match_found, expected_gain_select_string
        print(expected_gain_select_string)

        press_any_key_to_continue(serial_port)
        press_escape_key_to_continue(serial_port)
        
        clear_serial_input_buffer(serial_port)


def test_menu_assert_software_reset(serial_port, target_reset):
    """tests the software reset menu"""

    # Clear the serial input buffers
    clear_serial_input_buffer(serial_port)

    print('\nSoftware Reset (For respective channels 0 to n):')
    for dac_channel in range (0, number_of_dac_channels):
        # Call a function that selects the DAC channel menu
        select_dac_channel(serial_port, dac_channel)

        # Write a command to invoke DAC data register write menu
        send_serial_command(serial_port, menu_command['test_menu_write_and_update_dac'])

        # Write a code/data value from the local code/data list
        data_str = str(dac_channel_data_list[dac_channel][channel_data_val_indx]) + '\r' + '\n'
        send_serial_command(serial_port, data_str)

        press_any_key_to_continue(serial_port)

        # Write a command to invoke software reset menu
        send_serial_command(serial_port, menu_command['test_menu_assert_software_reset'])

        # Verify the result
        expected_software_reset_string = 'Software Reset Complete...'
        expected_string_match_found = False
        print()

        lines = serial_port.readlines(5000)
        for line in lines:
            if expected_software_reset_string in line.decode():
                expected_string_match_found = True
                break

        assert expected_string_match_found, expected_software_reset_string
        print(expected_software_reset_string)

        press_any_key_to_continue(serial_port)

        # Verify the string by envoking DAC readback menu
        send_serial_command(serial_port, menu_command['test_menu_dac_readback'])

        # Verify the result
        expected_data_readback_string = device_reset_data_list[dac_channel]
        expected_string_match_found = False
        print()

        lines = serial_port.readlines(5000)
        for line in lines:
            if expected_data_readback_string in line.decode():
                expected_string_match_found = True
                break

        assert expected_string_match_found, expected_data_readback_string
        print(expected_data_readback_string)
        
        press_any_key_to_continue(serial_port)
        clear_serial_input_buffer(serial_port)


def test_menu_assert_hardware_reset(serial_port, target_reset):
    """tests the hardware reset menu"""

    # Clear the serial input buffers
    clear_serial_input_buffer(serial_port)

    print('\nHardware Reset (For respective channels 0 to n):')
    for dac_channel in range (0, number_of_dac_channels):
        # Call a function that selects the DAC channel menu
        select_dac_channel(serial_port, dac_channel)

        # Write a command to invoke DAC data register write menu
        send_serial_command(serial_port, menu_command['test_menu_write_and_update_dac'])

        # Write a code/data value from the local code/data list
        data_str = str(dac_channel_data_list[dac_channel][channel_data_val_indx]) + '\r' + '\n'
        send_serial_command(serial_port, data_str)

        press_any_key_to_continue(serial_port)

        # Write a command to invoke hrdware reset menu
        send_serial_command(serial_port, menu_command['test_menu_assert_hardware_reset'])

        # Verify the result through output message string
        expected_hardware_reset_string = 'Hardware Reset Complete...'
        expected_string_match_found = False
        print()

        lines = serial_port.readlines(5000)
        for line in lines:
            if expected_hardware_reset_string in line.decode():
                expected_string_match_found = True
                break

        assert expected_string_match_found, expected_hardware_reset_string
        print(expected_hardware_reset_string)

        press_any_key_to_continue(serial_port)

        # Verify the string by invoking DAC readback menu
        send_serial_command(serial_port, menu_command['test_menu_dac_readback'])

        # Verify the result
        expected_data_readback_string = device_reset_data_list[dac_channel]
        expected_string_match_found = False
        print()

        lines = serial_port.readlines(5000)
        for line in lines:
            if expected_data_readback_string in line.decode():
                expected_string_match_found = True
                break

        assert expected_string_match_found, expected_data_readback_string
        print(expected_data_readback_string)
        
        press_any_key_to_continue(serial_port)
        clear_serial_input_buffer(serial_port)


#"""" Functions not part of the test script (without prefix test_) """"

def select_dac_channel(serial_port, dac_channel):
    # Write a command to display DAC channel selection menu
    send_serial_command(serial_port, menu_command['test_menu_dac_channel_selection'])

    # Write a command to select the DAC channel
    send_serial_command(serial_port, dac_channel_selection_list[dac_channel][channel_select_cmd_indx])

    press_any_key_to_continue(serial_port)
    press_escape_key_to_continue(serial_port)


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


def clear_serial_input_buffer(serial_port):
    # Reset/Clear the serial input buffer
    serial_port.reset_input_buffer()
    sleep(short_time)
