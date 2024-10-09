AD719x IIO Application
""""""""""""""""""""""

==================
Supported Hardware
==================

**Supported Devices:**

* `AD7190 <https://www.analog.com/en/products/ad7190.html>`_
* `AD7192 <https://www.analog.com/en/products/ad7192.html>`_
* `AD7193 <https://www.analog.com/en/products/ad7193.html>`_
* `AD7194 <https://www.analog.com/en/products/ad7194.html>`_
* `AD7195 <https://www.analog.com/en/products/ad7195.html>`_

**Supported Evaluation Boards:**

* `EVAL-AD7190 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad7190asdz.html>`_
* `EVAL-AD7192 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad7192asdz.html>`_
* `EVAL-AD7193 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad7193asdz.html>`_
* `EVAL-AD7195 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad7195asdz.html>`_

**Supported Carrier Boards:**

* `SDP-K1 With Mbed <https://os.mbed.com/platforms/SDP_K1/>`_

============
Introduction
============

This page gives an overview of using the ARM platforms supported (default is Mbed) 
firmware example with Analog Devices AD719x Evaluation board and SDP-K1 controller board. 
This example code leverages the ADI developed IIO (Industrial Input Output) ecosystem to 
evaluate the AD719x device by providing a device debug and data capture support.
The code provides support to MBED and STM32 platforms.

The respective build guides for each of the platforms
can be found in the further sections. The active platform can be chosen by selecting the appropriate 
value for the ACTIVE_PLATFORM macro in the app_config.h (Default is mbed)

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

Required: SDP-K1 (or alternative Mbed enabled controller board, or an STM32 board), EVAL-AD719x board
and USB cable.

Connect the EVAL-AD719x board to SDP-K1 board (or any other Mbed enabled controller 
board or an equivalent STM32 board) using jumper wires. Connect SDP-K1 board to the PC using the USB cable. 


   .. image:: /source/projects/ad719x_iio/ad719x_hardware_connection.png
      :width: 600


===============
Jumper Settings
===============

**SDP-K1:**

Connect the VIO_ADJUST jumper on the SDP-K1 board to 3.3V position to drive SDP-K1 GPIOs at 3.3V

**EVAL-AD719X:**

* Stack the EVAL-AD719X-ASDZ on the Arduino connectors of the SDP-K1 board.
* Connect a male-to-male jumper wire between D8 and D12 on Arduino connectors.
* Set the LK7 and LK8 jumper headers to 3.3V.
* Set LK12 jumper header to position

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
