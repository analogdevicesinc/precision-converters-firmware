LTC2672 IIO Application
"""""""""""""""""""""""

==================
Supported Hardware
==================

**Supported Devices:**

* `LTC2662 <https://www.analog.com/en/products/ltc2662.html>`_
* `LTC2672 <https://www.analog.com/en/products/ltc2672.html>`_

**Supported Evaluation Boards:**

* `DC2903 <https://www.analog.com/en/design-center/evaluation-hardware-and-software/evaluation-boards-kits/DC2903A.html>`_
* `EVAL-LTC2662-ARDZ <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ltc2662.html>`_
* `EVAL-LTC2672-ARDZ <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ltc2672.html>`_

**Supported Carrier Boards:**

* `SDP-K1 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/sdp-k1.html#eb-overview>`_

============
Introduction
============

This page gives an overview of using the ARM platforms supported
firmware example with Analog Devices DC2903A / Eval-LTC2662 / Eval-LTC2672 Evaluation boards and SDP-K1 controller board.
This example code leverages the ADI developed IIO (Industrial Input Output) ecosystem to
evaluate the DC29003A or Eval-LTC2662 or Eval-LTC2672 device by providing a device configuration and data streaming support.

   .. image:: /source/tinyiiod/app_interface.png
      :width: 350

The interface used for communicating with PC based IIO clients is either Virtual Serial Or UART. 
IIO Firmware leverages the ADI created no-os and platform driver software layers
to communicate with IIO device.

.. note::

   This code has been developed and tested on the SDP-K1 Controller Board with
   Arduino headers. However, the same code can be used with minimal modifications
   on any STM32 enabled board which has Arduino Header support on it.

.. Useful links Section

.. include:: /source/useful_links_stm32.rst

====================
Hardware Connections
====================
**LTC2662 / LTC2672:**

Required: SDP-K1 (or an STM32 board), EVAL-LTC2662 or EVAL-LTC2672 board and USB cable.

Connect the EVAL-LTC2662/EVAL-LTC2672 board to SDP-K1 board (or an equivalent STM32 board). Connect controller board to the PC using the USB cable.

   .. image:: /source/projects/ltc2672_iio/ltc2672_connection_diagram.png
      :width: 600

**DC2903A-A:**

   .. image:: /source/projects/ltc2672_iio/ltc2672_hardware_connections.jpg
      :width: 700

The connections to be made between the SDP-K1 and the DC2903A-A are as follows:

+-----------------------+-----------------------+
|SDP-K1 Arduino Header  | DC2903A Through-Hole  |
+-----------------------+-----------------------+
|SCLK/D13               | SCK                   |
+-----------------------+-----------------------+
|MISO/D12               | SDO                   |
+-----------------------+-----------------------+
|MOSI/D11               | SDI                   |
+-----------------------+-----------------------+
|CS/D10                 | CS                    |
+-----------------------+-----------------------+
|GND                    | GND                   |
+-----------------------+-----------------------+
|IOREF                  | J1 pin 2              |
+-----------------------+-----------------------+

   .. image:: /source/projects/ltc2672_iio/ltc2672_interface_diagram.jpg
      :width: 800

The power to the evaluation board should be supplied through the on-board turret connections within the following supply range:

+-----------------------+-----------------------+
|Pin                    | Voltage Supply        |
+-----------------------+-----------------------+
|VCC                    | 2.85V to 5.5V         |
+-----------------------+-----------------------+
|V-                     | -5.5V to 0V           |
+-----------------------+-----------------------+
|GND                    | GND                   |
+-----------------------+-----------------------+
|IOVCC                  | 1.71 to VCC           |
+-----------------------+-----------------------+

===============
Jumper Settings
===============

**SDP-K1 :**

Connect the VIO_ADJUST jumper on the SDP-K1 to 3.3V position
to drive SDP-K1 GPIOs at 3.3V.

**DC2903A-A / EVAL-LTC2662 / LTC2672:**

Please refer to the user guide for the jumper connections on the EVAL-LTC2662/LTC2672 board

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
