AD777x IIO Application
----------------------

==================
Supported Hardware
==================

**Supported Devices:**

* `AD7770 <https://www.analog.com/en/products/ad7770.html>`_ 
* `AD7771 <https://www.analog.com/en/products/ad7771.html>`_ 
* `AD7779 <https://www.analog.com/en/products/ad7779.html>`_ 

**Supported Carrier Boards:**

* `SDP-K1 With Mbed Platform <https://os.mbed.com/platforms/SDP_K1/>`_
* `Nucleo-L552ZEQ With STM32 Platform <https://os.mbed.com/platforms/ST-Nucleo-L552ZE-Q/>`_

============
Introduction
============

This page gives an overview of using the ARM platforms supported (default is Mbed) firmware application with Analog Devices AD777x Evaluation board(s) and SDP-K1/Nucleo-L552ZEQ controller board. 
This example code leverages the ADI developed IIO (Industrial Input Output) ecosystem to evaluate the AD777x family of devices by providing a device debug and data capture support.
The code provides support to MBED and STM32 platforms.

The respective build guides for each of the platforms
can be found in the further sections. The active platform can be chosen by selecting the appropriate 
value for the ACTIVE_PLATFORM macro in the app_config.h (Default is mbed)

Example: #define ACTIVE_PLATFORM STM32_PLATFORM would enable the firmware to compile using the STM32 platform.

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

Required: SDP-K1 (or alternative Mbed enabled controller board, or an STM32 board ), EVAL-AD777x board
and USB cable.

Connect the EVAL-AD777x board to SDP-K1 board (or any other Mbed enabled controller 
board or an equivalent STM32 board). Connect controller board to the PC using the USB cable. 

   .. image:: /source/projects/ad777x_iio/stm_diagram_updated.png
      :width: 600

===============
Jumper Settings
===============

**SDP-K1:**

Connect the VIO_ADJUST jumper on the SDP-K1 board to 3.3V position to drive SDP-K1 GPIOs at 3.3V

**Nucleo L552ZEQ for STM32 platform:**

The Nucleo L552ZEQ board has been used to develop and test the STM32 firmware.Please refer to the board user guide `Nucleo-L552ZEQ User Manual <https://www.st.com/content/ccc/resource/technical/document/user_manual/group1/ad/a4/bd/5e/14/15/4e/e8/DM00615305/files/DM00615305.pdf/jcr:content/translations/en.DM00615305.pdf>`_

**EVAL-AD777x:**

* Please refer to the respective board user guide on the product page of the chosen device.

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
