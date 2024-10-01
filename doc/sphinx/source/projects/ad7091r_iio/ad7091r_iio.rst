AD7091R IIO Application
"""""""""""""""""""""""

==================
Supported Hardware
==================

**Supported Devices:**

* `AD7091R-8 <https://www.analog.com/en/products/ad7091r-8.html>`_ 

**Supported Evaluation Boards:**

* `EVAL-AD7091R-8ARDZ <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad7091r-8.html>`_ 

**Supported Carrier Boards:**

* `SDP-K1 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/sdp-k1.html#eb-overview>`_

============
Introduction
============

This page gives an overview of using the ARM platforms supported 
firmware example with Analog Devices AD7091r Evaluation board and SDP-K1 controller board. 
This example code leverages the ADI developed IIO (Industrial Input Output) ecosystem to 
evaluate the AD7091r device by providing a device debug and data capture support.

   .. image:: /source/tinyiiod/app_interface.png
      :width: 350

The interface used for communicating with PC based IIO clients is either Virtual Serial Or UART. 
IIO Firmware leverages the ADI created no-os and platform driver software layers
to communicates with IIO device.

.. note::

   This code has been developed and tested on the SDP-K1 Controller Board with
   Arduino headers. However, the same code can be used with minimal modifications
   on any STM32 board which has Arduino Header support on it. 

.. Useful links Section

.. include:: /source/useful_links_stm32.rst

====================
Hardware Connections
====================

Required: SDP-K1, EVAL-AD7091R-8ARDZ board
and USB cable.

Connect the EVAL-AD7091R-8ARDZ board to SDP-K1 board (or any STM32 controller 
board). Connect controller board to the PC using the USB cable. 


   .. image:: /source/projects/ad7091r_iio/ad7091r_hardware_interface.png
      :width: 600


===============
Jumper Settings
===============

**SDP-K1:**

Connect the VIO_ADJUST jumper on the SDP-K1 board to 3.3V position to drive SDP-K1 GPIOs at 3.3V

**EVAL-AD7091R:**

* Please refer to the respective board user guide on the product page of the chosen device.

.. note::
   To capture data using the existing firmware example, you will need to connect the arduino header pin ARDUINO UNO D2 (GP0_Busy pin) to the ARDUINO UNO D1 pin using a flywire.

.. Communication Interface section:

SDP-K1 is powered through USB connection from the computer. SDP-K1 MCU board
acts as a serial device when connected to PC, which creates a serial ports to connect to IIO
client application running on PC. The serial port assigned to a device can be seen
through the device manager for windows-based OS as shown below:

.. image:: /source/hardware/serial_ports_view.png
   :width: 350

.. note::

   The serial port naming is used differently on different operating systems.
   For example, Linux uses terms such as dev/ttyUSB* and Mac uses terms such as dev/tty.USB*.
   Please check serial port naming for your selected OS.

.. Project Build Section:
    
.. include:: /source/build/project_build_stm32.rst

.. IIO Ecosystem Section:
    
.. include:: /source/tinyiiod/iio_ecosystem.rst

.. IIO Firmware Structure

.. include:: /source/tinyiiod/iio_firmware_structure.rst

==============================
Configuring Sampling Frequency
==============================

Using the sampling_frequency iio attribute, the rate at which the converter samples the data can be controlled. In this case, it can go all the way from 1000SPS to 830KSPS in case of
SPI-DMA based data capture.
The attribute is set to maximum sampling rate supported by default by the platform and can be varied in the accepted ranges by writing to the it.

Based on the sampling_frequency input, the FW calculates the nearest possible period value (based on the prescalers, counter resolutions and other settings) that can be acheived on the counter
and returns back the same after setting it to the counter registers.

Here's a simple formula that calculates the same:
   .. image:: /source/projects/ad7091r_iio/sampling_frequency_calculation.png
      :width: 600

================================
Reference Voltage Configuration
================================

The reference voltage for AD7091R can be configured using the reference_sel and reference_value_volts iio attributes.
The attribute can be read and written to get/set the reference voltage value in mV.

When reference_sel is set to "internal", the reference_value_volts is fixed at 2500mV and the attribute returns the same.
When reference_sel is set to "external", the reference_value_volts attribute can be used to set the external reference voltage value in mV ranging from 1000mV (Vref min) to 3300mV (VDD)

================================================
Changing the default SPI clock frequency setting
================================================

The default SPI clock frequency in the firmware is set to 40MHz, lowering the system clock frequency of SDP-K1 to 160MHz from 180MHz to get the maximum throughput of 830KSPS ODR (Output data rate).
This is because of the bottleneck in data capture at 45MHz SPI clock rate using the SDP-K1.
The SPI clock frequency can be changed to the default settings that SDP-K1 supports: 45Meg/22.5Meg by doing the following changes in the ioc file.

Change the HCLK setting from 160 to 180 (max supported) in the clock configuration tab opening the ioc file in the STM32CubeMX
   .. image:: /source/projects/ad7091r_iio/clock_configuration.png
      :width: 600

Once the ioc file is changed, follow the build guide to build your project and generate executable file (.bin/.hex)

=======
Support
=======

Feel free to ask questions in the `EngineerZone <https://ez.analog.com/data_converters>`_
