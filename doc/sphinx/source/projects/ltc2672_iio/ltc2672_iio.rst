LTC2672 IIO Application
"""""""""""""""""""""""

==================
Supported Hardware
==================

**Supported Devices:**

* `LTC2672 <https://www.analog.com/en/products/ltc2672.html>`_

**Supported Evaluation Boards:**

* `DC2903 <https://www.analog.com/en/design-center/evaluation-hardware-and-software/evaluation-boards-kits/DC2903A.html>`_

**Supported Carrier Boards:**

* `SDP-K1 With Mbed <https://os.mbed.com/platforms/SDP_K1/>`_

============
Introduction
============

This guide gives an overview of using the IIO firmware application with Analog Devices 
DC2903A Evaluation board and SDP-K1 (or other compatible) MCU controller board, 
leveraging Mbed-OS as a primary software platform. This firmware application 
leverages the ADI developed IIO (Industrial Input Output) ecosystem to evaluate 
the LTC2672 (IIO) device by providing device configuration and data streaming support.

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

   .. image:: /source/projects/ltc2672_iio/ltc2672_hardware_connections.jpg
      :width: 700

The connections to be made between the SDP-K1 and the DC2903A-A are as follows:

**DC2903A-A:**

Connect the VIO_ADJUST jumper on the SDP-K1 board to 3.3V position to drive SDP-K1 GPIOs at 3.3V

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
