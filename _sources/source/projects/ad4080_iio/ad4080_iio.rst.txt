AD4080 IIO Application
""""""""""""""""""""""

==================
Supported Hardware
==================

**Supported Devices:**

* `AD4080 <https://www.analog.com/en/products/ad4080.html>`_ 

**Supported Evaluation Boards:**

* `EVAL-AD4080-ARDZ <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad4080ardz.html>`_

**Supported Carrier Boards:**

* `SDP-K1  <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/sdp-k1.html#eb-overview>`_
* `Nucleo-H563ZI  <https://www.st.com/en/evaluation-tools/nucleo-h563zi.html/>`_ (supports Quad-SPI)

============
Introduction
============

This page gives an overview of using the ARM platforms supported
firmware example with Analog Devices AD4080 Evaluation board and SDP-K1/Nucleo-H563ZI controller board. 
This example code leverages the ADI developed IIO (Industrial Input Output) ecosystem to 
evaluate the AD4080 device by providing a device debug and data capture support.

   .. image:: /source/tinyiiod/app_interface.png
      :width: 350

The interface used for communicating with PC based IIO clients is either Virtual Serial Or UART. 
IIO Firmware leverages the ADI created no-os and platform driver software layers
to communicate with IIO device.

.. note::

   This code has been developed and tested on the SDP-K1 Controller Board with
   Arduino headers and on the Nucleo-H563ZI Controller Board with extended ST Zio connector headers.
   However, the same code can be used with minimal modifications
   on any STM32 enabled board which has Arduino Header support on it. The extended
   ST Zio connector headers are necessary to support Quad-SPI.

.. Useful links Section

.. include:: /source/useful_links_stm32.rst

====================
Hardware Connections
====================

Required: SDP-K1 (or an STM32 board like Nucleo-H563ZI), EVAL-AD4080 board (requires an external power supply,
for which the cable and adapter are provided with the evaluation board kit) and USB cable.

Connect the EVAL-AD4080 board to SDP-K1 board (or an STM32 board). Connect controller board to the PC using the USB cable.

   .. image:: /source/projects/ad4080_iio/ad4080_connection_diagram.png
      :width: 600

===============
Solder Bridge and Jumper Settings
===============

**Nucleo-H563ZI:**

SB70 solder bridge should be in the 'ON' state (i.e. place a small amount of solder paste between the two pads to be bridged).

Connect the JP4 jumper on the Nucleo-H563ZI board to 3.3V position to drive Nucleo-H563ZI GPIOs at 3.3V.

**SDP-K1:**

Connect the VIO_ADJUST jumper on the SDP-K1 to 3.3V position to drive SDP-K1 GPIOs at 3.3V.

**EVAL-AD4080:**

To configure the evaluation board for Single-SPI mode, connect RJ15, RJ16 and RJ17 to position 'A' (default).
For Quad-SPI mode, connect them to position 'B'.
For more details on this and the jumper connections on the EVAL-AD4080 board, please refer to the user guide.

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

=======
Support
=======

Feel free to ask questions in the `EngineerZone <https://ez.analog.com/data_converters>`_
