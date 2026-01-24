AD5706r IIO Application
""""""""""""""""""""""""

==================
Supported Hardware
==================

**Supported Devices:**

* `AD5706r <https://www.analog.com/en/products/ad5706r.html>`_

**Supported Evaluation Boards:**

* `EVAL-AD5706r <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad5706r.html#eb-overview>`

**Supported Carrier Boards:**

* `SDP-K1 With STM32 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/sdp-k1.html>`_

============
Introduction
============

This guide gives an overview of using the IIO firmware application with Analog Devices 
AD5706R Evaluation board and SDP-K1 (or other compatible) MCU controller board,  
leveraging STM32 as a primary software platform. This firmware application 
leverages the ADI developed IIO (Industrial Input Output) ecosystem to evaluate 
the AD5706R (IIO) device by providing device configuration and data streaming support.

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

Required: SDP-K1 (or alternative STM32 enabled controller board), EVAL-AD5706r board (requires an external power supply) and USB cable.
Connect the EVAL-AD5706r board to SDP-K1 board (or any other STM32 enabled controller board). 
Connect SDP-K1 board to the PC using the USB cable.

   .. image:: /source/projects/ad5706_iio/ad5706_connection_diagram.png
      :width: 700

===============
Jumper Settings
===============

**SDP-K1:**

Connect the VIO_ADJUST jumper on the SDP-K1 board to 3.3V position to drive SDP-K1 GPIOs at 3.3V

**EVAL-AD5706R:**

Please refer to the user guide for the jumper connections on the EVAL-AD5706R board

.. Communication Interface section:

.. include:: /source/hardware/comm_interface.rst

.. Project Build Section:
    
.. include:: /source/build/project_build_stm32.rst

.. IIO Ecosystem Section:
    
.. include:: /source/tinyiiod/iio_ecosystem.rst

.. note::

    While sending DAC codes from any of IIO Clients to DAC, provide 1st channel data first 
    then 0th channel data.

.. IIO Firmware Structure

.. include:: /source/tinyiiod/iio_firmware_structure.rst

=======
Support
=======

Feel free to ask questions in the `EngineerZone <https://ez.analog.com/data_converters>`_
