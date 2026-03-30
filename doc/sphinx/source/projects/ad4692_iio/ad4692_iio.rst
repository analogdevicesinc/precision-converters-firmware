AD4692 IIO Application
""""""""""""""""""""""

==================
Supported Hardware
==================

**Supported Devices:**

* `AD4692 <https://www.analog.com/en/products/ad4692.html>`_ 

**Supported Evaluation Boards:**

* `EVAL-AD4692-ARDZ <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad4692-ardz.html>`_

**Supported Carrier Boards:**

* `SDP-K1 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/sdp-k1.html#eb-overview>`_

============
Introduction
============

This page gives an overview of using the ARM platforms supported firmware example with 
Analog Devices AD4692 Evaluation board and SDP-K1 controller board. This example code 
leverages the ADI developed IIO (Industrial Input Output) ecosystem to evaluate the 
AD4692 device by providing a device debug and data capture support.

The firmware provides support to STM32 platform. Highest sampling rate can be achieved 
using the SPI_DMA Mode in the STM32 platform.

   .. image:: /source/tinyiiod/app_interface.png
      :width: 350

The interface used for communicating with PC based IIO clients is either Virtual Serial Or UART. 
IIO Firmware leverages the ADI created no-os and platform driver software layers to communicate 
with IIO device.

.. Useful links Section

.. include:: /source/useful_links_stm32.rst

====================
Hardware Connections
====================

Required: SDP-K1 (or an STM32 board ), EVAL-AD4692 board and USB cable.

Connect the EVAL-AD4692 board to SDP-K1 board (or an equivalent STM32 board). Connect controller board to the PC using the USB cable. 

   .. image:: /source/projects/ad4692_iio/ad4692_connection_diagram.png
      :width: 600


===============
Jumper Settings
===============
	
**SDP-K1:**

Connect the VIO_ADJUST jumper on the SDP-K1 board to 3.3V position to drive SDP-K1 GPIOs at 3.3V

**EVAL-AD4692:**

Please refer to the board user manual for the jumper settings.

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
