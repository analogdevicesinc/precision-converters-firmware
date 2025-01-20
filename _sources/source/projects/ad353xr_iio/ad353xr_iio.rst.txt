AD3530R IIO Application
"""""""""""""""""""""""

==================
Supported Hardware
==================

**Supported Devices:**

* `AD3530R <https://www.analog.com/en/ad3530r.html>`_ 

**Supported Evaluation Boards:**

* `EVAL-AD3530RARDZ <https://www.analog.com/eval-ad3530r>`_ 

**Supported Carrier Boards:**

* `SDP-K1 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/sdp-k1.html#eb-overview>`_

============
Introduction
============

This page gives an overview of using the ARM platforms supported 
firmware example with Analog Devices AD3530r Evaluation board and SDP-K1 controller board. 
This example code leverages the ADI developed IIO (Industrial Input Output) ecosystem to 
evaluate the AD3530r device by providing a device debug and data capture support.

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

Required: SDP-K1, EVAL-AD3530RARDZ board
and USB cable.

Connect the EVAL-AD3530RARDZ board to SDP-K1 board (or any STM32 controller 
board). Connect controller board to the PC using the USB cable. 


   .. image:: /source/projects/ad353xr_iio/ad3530r_hardware_interface.png
      :width: 600


===============
Jumper Settings
===============

**SDP-K1:**

Connect the VIO_ADJUST jumper on the SDP-K1 board to 3.3V position to drive SDP-K1 GPIOs at 3.3V

**EVAL-AD3530R:**

* Please refer to the respective board user guide on the product page of the chosen device.

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

===================
Waveform Generation
===================

The Firmware supports two modes of data streaming using the SPI-DMA technique for Waveform generation from the IIO client

Configuring data streaming modes
--------------------------------

The IIO client can configure the data streaming mode by setting the "data_streaming_mode" iio attribute. The supported modes are:

1. Single Instruction mode

2. Streaming mode

Please refer to the data sheet for more details on the data streaming modes.

Configuring Sampling Frequency
-------------------------------

Using the "sampling_frequency" iio attribute, the rate at which the converter updates the data can be controlled. In this case, it can go all the way from 1000SPS to 570KSPS in case of Single Instruction mode of data streaming. In case of Streaming mode, the sampling frequency is fixed at 1.4MSPS and non-configurable.
The attribute is set to maximum sampling rate supported by default by the platform and can be varied in the accepted ranges by writing to the it.

Based on the sampling_frequency input, the FW calculates the nearest possible period value (based on the prescalers, counter resolutions and other settings) that can be acheived on the counter
and returns back the same after setting it to the counter registers.

Here's a simple formula that calculates the same:
   .. image:: /source/projects/ad353xr_iio/sampling_frequency_calculation.png
      :width: 600

When to use what mode?
-----------------------

Any combination of channels can be sequenced in case of Single Instruction Mode and sampling_frequency (update rate) can be varied as per the requirement. However, since every transaction is a single instruction (requires an address plus data phase), the maximum achievable sampling rate is limited to 570KSPS.

On the other hand, in case of Streaming mode, only serial channels are allowed to be sequenced (as the chip assumes the regiters to be contiguous in streaming mode) and the sampling frequency is fixed at 1.4MSPS (the highest throughput possible at a given SPI clok frequency). This mode is useful when the maximum sampling rate is required and the data is to be streamed continuously.

=======
Support
=======

Feel free to ask questions in the `EngineerZone <https://ez.analog.com/data_converters>`_
