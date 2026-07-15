AD2S1210 IIO Application
""""""""""""""""""""""""

==================
Supported Hardware
==================

**Supported Devices:**

* `AD2S1210 <https://www.analog.com/en/products/ad2s1210.html>`_

**Supported Evaluation Boards:**

* `EVAL-AD2S1210 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/ad2s1210.html>`_

**Interposer Board:**

* `AD2S1210 Interposer Board <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/ad2s1210.html>`_

**Supported Carrier Boards:**

* `SDP-K1 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/sdp-k1.html>`_

============
Introduction
============

This page gives an overview of using the ARM platforms supported (default is STM32) 
firmware example with Analog Devices AD2S1210 Evaluation board and SDP-K1 controller board. 
This example code leverages the ADI developed IIO (Industrial Input Output) ecosystem to 
evaluate the AD2S1210 device by providing a device debug and data capture support.

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

Required: SDP-K1, AD2S1210 Interposer Board, EVAL-AD2S1210 board and USB cable.
Plug in the EVAL-AD2S1210 board to 9V adapter and then connect to the AD2S1210 Interposer Board using sdp-120 headers. Connect the Interposer Board to the
SDP-K1 board using the Arduino connector.

   .. image:: /source/projects/ad2s1210_iio/ad2s1210_connection_diagram.png
      :width: 600

Connect the SDP-K1 board to the PC using the USB cable and the AD2S1210 EVB to the
provided AC 9V adapter.

===============
Jumper Settings
===============

**SDP-K1:**

Connect the VIO_ADJUST jumper on the SDP-K1 board to 3.3V position to drive SDP-K1 GPIOs at 3.3V

**EVAL-AD2S1210:**

* Please refer to the respective board user guide on the product page of the chosen device.

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
