AD738x IIO Application
""""""""""""""""""""""

==================
Supported Hardware
==================

**Supported Devices:**

* `AD7380 <https://www.analog.com/en/products/ad7380.html>`_

**Supported Evaluation Boards:**

* `EVAL-AD738xFMCZ <https://www.analog.com/en/design-center/evaluation-hardware-and-software/evaluation-boards-kits/EVAL-AD738xFMCZ.html>`_

**Supported Carrier Boards:**

* `SDP-K1 With Mbed <https://os.mbed.com/platforms/SDP_K1/>`_

============
Introduction
============

This guide gives an overview of using the IIO firmware application with Analog Devices 
AD738x Evaluation board and SDP-K1 (or other compatible) MCU controller board, 
leveraging Mbed-OS as a primary software platform. This firmware application 
leverages the ADI developed IIO (Industrial Input Output) ecosystem to evaluate 
the AD738x (IIO) device by providing device configuration and data capture support.

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

Required: SDP-K1 (or alternative Mbed enabled controller board), EVAL-AD738x board, 
9V adapter and USB cable.

Connect the EVAL-AD7380 board to SDP-K1 board (or any other Mbed enabled controller 
board) using jumper wires. Connect SDP-K1 board to the PC using the USB cable. 
AD738x FMC EVB is powered through external 9-12V DC adapter.

===============
Jumper Settings
===============

**SDP-K1:**

Connect the VIO_ADJUST jumper on the SDP-K1 board to 3.3V position to drive SDP-K1 GPIOs at 3.3V

**EVAL-AD738xFMCZ:**

Use default board settings.

.. Communication Interface section:

.. include:: /source/hardware/comm_interface.rst

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
