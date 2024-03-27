===========================
IIO Data Streaming for DACs
===========================

Introduction
============

IIO clients can be used to load and stream data to DAC devices using an IIO 
firmware application developed for that device. Using this firmware, a user 
can configure various settings on the device and load the data using the DAC 
Data Manager tab. The data is transmitted using Virtual Serial or UART serial 
link. The input data to be streamed can be loaded in a binary data file or a MAT
file (.mat) format. 

The channels for which data is to be streamed can be selected from the GUI window, 
and the number of samples per channel will be taken from the input data file provided. 
So, for an 8-channel DAC, if all 8 channels are selected for streaming, the input 
file should contain 3200 samples, considering 400 samples per channel.

IIO client allows data to be streamed to the iio device in two ways: either cyclic 
streaming enabled or disabled. When the cyclic buffer streaming is 
enabled, the buffer repeats itself, and the same data is streamed over and over 
indefinitely until stopped.

Data Streaming to IIO Device
============================

The DAC Data Manager tab on the IIO Oscilloscope allows streaming data to supported iio devices.
The input data file can be a binary file (.bin) or a MAT file (.mat).

.. note::

   DAC Data Manager is not available in versions below 0.15. To use the Data Manager, 
   the IIO Oscilloscope application must be upgraded to the most recent versions.

Creating binary files for data streaming
========================================

:doc:`pyadi-iio: Python interfaces </source/python/python>`
The PyADI module allows users to create binary data files from the input data using the
tx class. The CLI (Command Line Interface) script adistream.py allows the user to 
create various types of waveforms like sine, triangular, pwm, etc and store them to a 
binary file.

Loading and Streaming the Data
==============================

On the IIO Oscilloscope, open the DAC Data Manager tab as indicated below 
after connecting to the iio device.

   .. image:: /source/iio_osc/iio_osc_data_manager.jpg
      :width: 600

Click on the File Selection section to input the wavefrom data binary file created.
Copy the binary file path and paste it in the Location section, then click Open.

   .. image:: /source/iio_osc/iio_osc_file_section.jpg
      :width: 600

.. note::
   
   The input data file shall contain data according to the number of active channels 
   enabled, and otherwise may result in incorrrect data being streamed.

Once the required channels are enabled, use the cyclic buffer checkbox to enable 
or disable cyclic streaming of data. Click the Load button to load the data. The 
message "Waveform loaded successfully" shows up if the data is loaded correctly. 

   .. image:: /source/iio_osc/iio_osc_load_data.jpg
      :width: 600

.. note::
   
   The Debug tab should not be accessed when streaming data, as this would 
   impact data streaming. All uses same communication bus to access the data which 
   could result into access/busy conflicts during data streaming.

If Enable/Disable cyclic buffer is unchecked, streaming stops once all the data from
the file is loaded, and if the cyclic buffer checkbox is checked, "Stop buffer transmission" 
button needs to be clicked to end the data streaming.

===================================================
Limitations of Data Streaming Using IIO Application
===================================================

**DAC Sampling/Update Rate:**

The sampling rate or update rate defines the maximum rate/speed at which data can 
be loaded and updated onto the DAC output using the IIO firmware application. For 
DACs, typical time to update single sample is defined as:

Time to update single sample: DAC data write time over digital interface + DAC conversion & output response time

Data update rate is also limited because of additional overhead in the MCU firmware, 
such as interrupt context switching time, SPI (digital interface) clock frequency, 
MCU clock speed, etc.

The "AD579X IIO Application" is used as a reference here. 
For AD5790, which is a one-channel VDAC, the update rate is 71.4 KSPS. So, if a 
sine wave with 200 data points is to be streamed, the output frequency 
that can be seen will be 357 Hz (fo = fs/np = Sample rate/number of data points = 
71.4K/200). The effective output frequency might also vary based on the number of 
channels enabled on the device.


**Data Transmission Rate (serial link) of MCU:**

This is the rate or speed at which data can be transmitted from the IIO client over 
the serial link (e.g. UART or Virtual serial). This especially needs to be considered 
when cyclic buffer streaming is disabled and the buffer data needs to be transferred 
from the client in real-time for a continuous update. 


**Buffer size limitations in the firmware:**

Size of data buffer on the firmware is always restricted due to MCU memory size. 
The required number of samples to be streamed from the IIO client application 
therefore must always be less than the total size of data buffer. If the 
required streaming samples number is more than the size of data buffer, IIO 
firmware returns negative error code to IIO client which then terminates the 
data streaming request.

Buffer size can be increased to larger value by making use of internal/external RAMs. 
For example, SDP-K1 MCU board has 16Mbytes of onboard SDRAM, which allows larger data 
buffer size in the firmware. By enabling SDRAM in the SDP-K1 targeted Mbed firmware 
(app_config.h file), the data buffer size can be increased to 16Mbytes.
