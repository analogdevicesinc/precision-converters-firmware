AD7091R IIO Application
"""""""""""""""""""""""

==================
Supported Hardware
==================

**Supported Devices:**

* `AD7091R-8 <https://www.analog.com/en/products/ad7091r-8.html>`_ 

**Supported Evaluation Boards:**

* `EVAL-AD7091R-8ARDZ <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad7091r.html>`_ 

**Supported Carrier Boards:**

* `SDP-K1 With Mbed <https://os.mbed.com/platforms/SDP_K1/>`_

============
Introduction
============

This page gives an overview of using the ARM platforms supported (default is Mbed) 
firmware example with Analog Devices AD7091r Evaluation board and SDP-K1 controller board. 
This example code leverages the ADI developed IIO (Industrial Input Output) ecosystem to 
evaluate the AD7091r device by providing a device debug and data capture support.

   .. image:: /source/tinyiiod/app_interface.png
      :width: 350

The interface used for communicating with PC based IIO clients is either Virtual Serial Or UART. 
IIO Firmware leverages the ADI created no-os and platform driver software layers
to communicates with IIO device.

.. SDP-K1 Mbed Section

.. include:: /source/tinyiiod/sdp_k1_mbed.rst

.. Useful links Section

.. include:: /source/useful_links.rst

====================
Hardware Connections
====================

Required: SDP-K1 (or alternative Mbed enabled controller board), EVAL-AD7091R-8ARDZ board
and USB cable.

Connect the EVAL-AD7091R-8ARDZ board to SDP-K1 board (or any other Mbed enabled controller 
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
   Anything?

.. Communication Interface section:

.. include:: /source/hardware/comm_interface.rst

.. Project Build Section:
    
.. include:: /source/build/project_build.rst

.. IIO Ecosystem Section:
    
.. include:: /source/tinyiiod/iio_ecosystem.rst

.. IIO Firmware Structure

.. include:: /source/tinyiiod/iio_firmware_structure.rst

==============================
Configuring Sampling Frequency
==============================

Using the sampling_frequency iio attribute, the rate at which the converter samples the data can be controlled. In this case, it can go all the way from 1000SPS to 930KSPS in case of
SPI-DMA based data capture.
The attribute is set to maximum sampling rate supported by default by the platform and can be varied in the accepted ranges by writing to the it.

Based on the sampling_frequency input, the FW calculates the nearest possible period value (based on the prescalers, counter resolutions and other settings) that can be acheived on the counter
and returns back the same after setting it to the counter registers.

Here's a simple formula that calculates the same:
   .. image:: /source/projects/ad7091r_iio/sampling_frequency_calculation.png
      :width: 600

=======
Support
=======

Feel free to ask questions in the `EngineerZone <https://ez.analog.com/data_converters>`_
