import pytest
import serial
from time import sleep
import re

# could make these fixtures if there is a need to share them more widely
short_time = 0.1
long_time = 1


#Key inputs correponding to each sensor
sensor_list = {'A': 'AD590 ( On-board Sensor )', 'B': 'AD590 ( External Remote Sensor )'}

#List of possible outputs
regex_list = [r"Temperature: (\d*)",r"OVER Voltage",r"UNDER Voltage",r"Conversion still"]

#===========================================================================================
def test_main_menu(serial_port, target_reset):
    """Tests the main menu header"""

    expect_match_string = b'EVAL-AD590-ARDZ Demonstration Program'
    expect_match_string_found = False
    
    #clears the serial buffer
    serial_port.flushOutput()
    sleep(short_time)
    
    # Send the ESC command, to get back to main menu
    serial_port.write(b'\x1b')
    sleep(short_time)

    # Read a large number of lines in case VT100 clear screen not used
    # port must have been opened with a timeout for this to work
    lines = serial_port.readlines(300)

    for line in lines:
        if expect_match_string in line:
            expect_match_string_found = True
            print()
            print(line.decode())
            break

    assert expect_match_string_found

    sleep(short_time)
    
#===========================================================================================
@pytest.mark.parametrize("sensor_select",['A','B'])
def test_sample_continuous(serial_port, target_reset,sensor_select):
    """Tests the continuous converted samples from all the 4 sensors"""
    
    #clears the serial buffer
    serial_port.flushOutput()
    sleep(short_time)
    
    # Send the Sample Menu command, wait for input to arrive
    serial_port.write(sensor_select.encode('utf-8'))
    sleep(short_time)
    
    # clear any pending input
    serial_port.reset_input_buffer()
    
    # Send the Continuous Conversion - Stream Menu command, wait for the output
    serial_port.write(b'1')
    sleep(long_time)

    # Read lines to gather all the lines on console
    lines = serial_port.readlines(300)
    
    expect_match_string_found = False

    for line in lines:
        #iterates through the list of possible output strings
        for i in range(len(regex_list)):
            p = re.search(regex_list[i], line.decode())
            if p:
                expect_match_string_found = True
                break

        if(expect_match_string_found == True):
            if (i == 0):
                #check for the temperature range
                temperature = p.group(1)
                assert (1 <= int(temperature) <= 50), "Error reading temperature!!"
                print('\nTemperature:', temperature)
                break
            
            assert not((i==1) or (i==2)), "%s Detected on %s " \
                    %(str(regex_list[i]),str(sensor_list[sensor_select]))
            assert not(i==3), "Device not connected properly!"
            break
     
    # Assert if none of the possible outputs is matched     
    assert expect_match_string_found
    
    # Press escape key to return operation type selection menu
    serial_port.write(b'\x1b')
    sleep(short_time)
    
    # Press escape key to return sensor selection menu
    serial_port.write(b'\x1b')
    sleep(short_time)
 
#=========================================================================================== 
@pytest.mark.parametrize("sensor_select",['A','B'])
def test_sample_interval(serial_port, target_reset,sensor_select):
    """Tests the samples converted after a particular duration from all the 4 sensors"""

    #clears the serial buffer
    serial_port.flushOutput()
    sleep(short_time)
    
    # Send the Sample Menu command, wait for input to arrive
    serial_port.write(sensor_select.encode('utf-8'))
    sleep(short_time)
    
    # clear any pending input
    serial_port.reset_input_buffer()
    
    # Send the inverval sample command, wait for input to arrive
    serial_port.write(b'2')
    sleep(long_time)
    
    # Send the command to select 10 samples, wait for input to arrive
    serial_port.write(b'10')
    serial_port.write(b'\r\n')
    sleep(short_time)

    # Send the command to select 500 miliseconds as duration
    serial_port.write(b'0.5')
    serial_port.write(b'\r\n')
    sleep(long_time)

    # Read lines to gather all the lines on console
    lines = serial_port.readlines(400)

    expect_match_string_found = False

    for line in lines:
        #iterates through the list of possible output strings
        for i in range(len(regex_list)):
            p = re.search(regex_list[i], line.decode('utf-8'))
            if p:
                expect_match_string_found = True
                break
            
        if(expect_match_string_found == True):
            if (i == 0):
                #check for the temperature range
                temperature = p.group(1)
                assert (1 <= int(temperature) <= 50), "Error reading temperature!!"
                print('\nTemperature:', temperature)
                break
            
            assert not((i==1) or (i==2)), "%s Detected on %s " \
                    %(str(regex_list[i]),str(sensor_list[sensor_select]))
            assert not(i==3), "Device not connected properly!"
            break
    
    # Assert if none of the possible outputs is matched 
    assert expect_match_string_found

    # Press escape key to return operation type selection menu
    serial_port.write(b'\x1b')
    sleep(short_time)
    
    # Press escape key to return sensor selection menu
    serial_port.write(b'\x1b')
    sleep(short_time)
    