AD4170 IIO Application
""""""""""""""""""""""

==================
Supported Hardware
==================

**Supported Devices:**

* `AD4170 <https://www.analog.com/en/products/ad4170-4.html>`_ 
* `AD4190 <https://www.analog.com/en/products/ad4190-4.html>`_ 

**Supported Carrier Boards:**

* `SDP-K1 With Mbed Platform <https://os.mbed.com/platforms/SDP_K1/>`_
* `Nucleo-H563ZI With STM32 Platform <https://www.st.com/resource/en/reference_manual/rm0481-stm32h563h573-and-stm32h562-armbased-32bit-mcus-stmicroelectronics.pdf>`_

============
Introduction
============

This page gives an overview of using the ARM platforms supported (default is Mbed) 
firmware example with Analog Devices AD4170 Evaluation board and SDP-K1 controller board. 
This example code leverages the ADI developed IIO (Industrial Input Output) ecosystem to 
evaluate the AD4170 device by providing a device debug and data capture support.
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

Required: SDP-K1 (or alternative Mbed enabled controller board, or an STM32 board ), EVAL-AD4170 board
and USB cable.

Connect the EVAL-AD4170 board to SDP-K1 board (or any other Mbed enabled controller 
board or an equivalent STM32 board). Connect controller board to the PC using the USB cable. 

   .. image:: /source/projects/ad4170_iio/ad4170_connection_diagram.png
      :width: 600

===============
Jumper Settings
===============

SDP-K1 / Nucleo-H563ZI :

Connect the VIO_ADJUST jumper on the SDP-K1 / JP4 on Nucleo-H563ZI board to 3.3V position 
to drive SDP-K1 / Nucleo-H563ZI GPIOs at 3.3V

**EVAL-AD4170:**

In case the firmware is being used with the SDP-K1 in the STM32 platform with SPI DMA Mode of operation,
an external jumper needs to be connected between D0 and D1 of the Arduino header of the SDP-K1

Please refer to the user guide for the jumper connections on the EVAL-AD4170 board

.. warning::
   
   Please make sure to enable/disable the channels of interest via the "system_config" attribute as shown below.
   
.. image:: /source/projects/ad4170_iio/ad4170_channel_enable.gif


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
