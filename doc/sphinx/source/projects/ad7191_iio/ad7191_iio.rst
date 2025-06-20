AD7191 IIO Application
""""""""""""""""""""""

==================
Supported Hardware
==================

**Supported Devices:**

* `AD7191 <https://www.analog.com/en/products/ad7191.html>`_

**Supported Evaluation Boards:**

* `EVAL-AD7191 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad7191asdz.html>`_

**Supported Carrier Boards:**

* `SDP-K1 With STM32 <https://os.mbed.com/platforms/SDP_K1/>`_

============
Introduction
============

This page gives an overview of using the ARM platforms supported 
firmware example with Analog Devices AD7191 Evaluation board and SDP-K1 controller board. 
This example code leverages the ADI developed IIO (Industrial Input Output) ecosystem to 
evaluate the AD7191 device by providing a device debug and data capture support.

   .. image:: /source/tinyiiod/app_interface.png
      :width: 600

The interface used for communicating with PC based IIO clients is either Virtual Serial Or UART. 
IIO Firmware leverages the ADI created no-os and platform driver software layers
to communicates with IIO device.

.. note::

   This code has been developed and tested on the SDP-K1 Controller Board with
   Arduino headers. However, the same code can be used with minimal modifications
   on any STM32 enabled board which has Arduino Header support on it. 

.. Useful links Section

.. include:: /source/useful_links.rst

====================
Hardware Connections
====================

Required: SDP-K1 (or an STM32 board ), EVAL-AD7191 board
and USB cable.

Connect the EVAL-AD7191 board to SDP-K1 board (or an STM32 board) using jumper wires. Connect SDP-K1 board to the PC using the USB cable. 


   .. image:: /source/projects/ad7191_iio/ad7191_hardware_connection.png
      :width: 500


===============
Jumper Settings
===============

**SDP-K1:**

Connect the VIO_ADJUST jumper on the SDP-K1 board to 3.3V position to drive SDP-K1 GPIOs at 3.3V

**EVAL-AD7191:**

* Stack the EVAL-AD7191-ASDZ on the Arduino connectors of the SDP-K1 board.
* Connect a male-to-male jumper wire between D3 and D12 on Arduino connectors.
* Set the LK7 and LK8 jumper headers to 3.3V.

=======================
Communication Interface
=======================

SDP-K1 is powered through USB connection from the computer. SDP-K1 MCU board 
acts as a serial device when connected to PC, which creates a serial ports to connect to IIO 
client application running on PC. The serial port assigned to a device can be seen 
through the device manager for windows-based OS as shown below:

.. image:: /source/hardware/serial_ports_view.png
   :width: 350

.. Project Build Section:
    
.. include:: /source/build/project_build.rst

.. IIO Ecosystem Section:
    
.. include:: /source/tinyiiod/iio_ecosystem.rst

.. IIO Firmware Structure

.. include:: /source/tinyiiod/iio_firmware_structure.rst

=======
Support
=======

Feel free to ask questions in the `EngineerZone <https://ez.analog.com/data_converters>`_
