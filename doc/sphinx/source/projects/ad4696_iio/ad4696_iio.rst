AD4696 IIO Application
----------------------

==================
Supported Hardware
==================

**Supported Devices:**

* `AD4695 <https://www.analog.com/en/products/ad4695.html>`_ 
* `AD4696 <https://www.analog.com/en/products/ad4696.html>`_ 

**Supported Evaluation Boards:**

* `EVAL-AD4696FMCZ <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad4696.html>`_

**Supported Carrier Boards:**

* `SDP-K1 With Mbed Platform <https://os.mbed.com/platforms/SDP_K1/>`_

============
Introduction
============

This page gives an overview of using the ARM platforms supported (default is Mbed) 
firmware example with Analog Devices AD4696 Evaluation board and SDP-K1 controller board. 
This example code leverages the ADI developed IIO (Industrial Input Output) ecosystem to 
evaluate the AD4696 device by providing a device debug and data capture support.

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

Required: SDP-K1 (or alternative Mbed enabled controller board, or an STM32 board ), EVAL-AD4696 board
and USB cable.

Connect the EVAL-AD4696 board to SDP-K1 board (or any other Mbed enabled controller 
board or an equivalent STM32 board). Connect controller board to the PC using the USB cable. 

   .. image:: /source/projects/ad4696_iio/ad4696_connection_diagram.png
      :width: 600


===============
Jumper Settings
===============

**Power Connections**

* Connect a 12V (1A max) DC power supply to board through VPWR and GND4 pin.
* Connect the VCC_HOST pin of the AD4696 to the 3.3V supply.
* Note: Make sure to connect the 12V DC power supply to VPWR before supplying 
  3.3v to the VCC_HOST pin to adhere to the power-supply-sequence requirement
	
**SDP-K1:**

Connect the VIO_ADJUST jumper on the SDP-K1 board to 3.3V position to drive SDP-K1 GPIOs at 3.3V

**EVAL-AD4696:**

.. note::
   Make below jumper settings on the board. Refer `EVAL-AD4696-FMCZ User Manual <https://www.analog.com/en/design-center/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad4696.html#eb-overview>`_  for more details.

+------+----------+-------------------------------------------------+-------------------------------------------------+
|Jumper| Position | Functionality                                   | Comment                                         | 
+------+----------+-------------------------------------------------+-------------------------------------------------+
|      |          |                                                 | Unipolar Mode: Change                           |
|JP6   | B        | Connects the COM pin of the ADC to ground       | Pseudo Bipolar Mode: Change to A to connect     |
|      |          | Or the output of the VREF/2 buffer              | the COM pin to VREF/2 V                         |
+------+----------+-------------------------------------------------+-------------------------------------------------+
|      |          |                                                 | Change to pos A to connect CS_N and CNV signals,|
|JP31  | B        | Connects the CNV and CS_N signals.By default    | as our firmware makes use of PWM signals to     |
|      |          | CNV and CS_N signals aren't connected togther   | manual trigger the conversion.                  |
+------+----------+-------------------------------------------------+-------------------------------------------------+

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

Please Feel free to ask questions in the `EngineerZone <https://ez.analog.com/data_converters>`_


