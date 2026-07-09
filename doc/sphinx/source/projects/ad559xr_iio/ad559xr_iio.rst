AD559xr IIO Application
""""""""""""""""""""""

==================
Supported Hardware
==================

**Supported Devices:**

* `AD5592r <https://www.analog.com/en/products/ad5592r.html>`_ 
* `AD5593r <https://www.analog.com/en/products/ad5593r.html>`_ 

**Supported Evaluation Boards:**

* `EVAL-AD5592R-PMDZ <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad5592r-pmdz.html>`_
* `EVAL-AD5593R-PMDZ <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad5593r-pmdz.html>`_

**Interposer Board:**

* `PMD-ARD-INT-LCZ <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/pmd-ard-int-lcz.html>`_

**Supported Carrier Boards:**

* `SDP-K1 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/sdp-k1.html>`_

============
Introduction
============

This page gives an overview of using the ARM platforms supported (default is STM32) 
firmware example with Analog Devices AD559xr Evaluation board and SDP-K1 controller board. 
This example code leverages the ADI developed IIO (Industrial Input Output) ecosystem to 
evaluate the AD559xr device by providing a device debug and data capture support.

   .. image:: /source/tinyiiod/app_interface.png
      :width: 350

The interface used for communicating with PC based IIO clients is either Virtual Serial Or UART. 
IIO Firmware leverages the ADI created no-os and platform driver software layers
to communicates with IIO device.

.. note::

   This code has been developed and tested on the SDP-K1 Controller Board with
   Arduino headers. However, the same code can be used with minimal modifications
   on any STM32 enabled board which has Arduino Header support on it.

.. Useful links Section

.. include:: /source/useful_links_stm32.rst

====================
Hardware Connections
====================

Required: SDP-K1 (or an STM32 board), PMD-ARD-INT-LCZ interposer board, EVAL-AD559xr PMDZ board
and USB cable.

Connect the EVAL-AD559xr board to SDP-K1 board (or an equivalent STM32 board) via the interposer board. 
Connect controller board to the PC using the USB cable. 

   .. image:: /source/projects/ad559xr_iio/ad559xr_connection_diagram.png
      :width: 600


===============
Jumper Settings
===============

**SDP-K1:**

Connect the VIO_ADJUST jumper on the SDP-K1 to 3.3V position to drive SDP-K1 GPIOs at 3.3V

**EVAL-AD559xr:**

* Stack the PMD-ARD-INT-LCZ on the Arduino connectors of the SDP-K1 board .
* Power up the SDP-K1 and then connect the AD559x using the PMOD headers.
* You need to use SPI1 PMOD header for ad5592r and I2C PMOD for AD5593r.

Please refer to the user guide for the jumper connections on the EVAL-AD5592R/AD5593R and the interposer board.

.. Communication Interface section:

.. include:: /source/hardware/comm_interface.rst

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
