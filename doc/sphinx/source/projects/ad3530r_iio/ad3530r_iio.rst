AD3530R IIO Application
"""""""""""""""""""""""

==================
Supported Hardware
==================

**Supported Devices:**

* `AD3530R <https://www.analog.com/en/products/TODO.html>`_ 

**Supported Evaluation Boards:**

* `EVAL-AD3530RARDZ <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-todo.html>`_ 

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


   .. image:: /source/projects/ad3530r_iio/ad3530r_hardware_interface.png
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

#TODO: Waveform gen
#TODO: Boardlevel attributes and techniques

=======
Support
=======

Feel free to ask questions in the `EngineerZone <https://ez.analog.com/data_converters>`_
