AD579X IIO Application
""""""""""""""""""""""

==================
Supported Hardware
==================

**Supported Devices:**

* `AD5790 <https://www.analog.com/en/products/ad5790.html>`_

* `AD5791 <https://www.analog.com/en/products/ad5791.html>`_

* `AD5780 <https://www.analog.com/en/products/ad5780.html>`_

* `AD5781 <https://www.analog.com/en/products/ad5781.html>`_

* `AD5760 <https://www.analog.com/en/products/ad5760.html>`_

**Supported Evaluation Boards:**

* `EVAL-AD5781ARDZ <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad5781ardz.html>`_

* `EVAL-AD5791ARDZ <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad5791ardz.html>`_

**Supported Carrier Boards:**

* `SDP-K1  <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/sdp-k1.html#eb-overview>`_

============
Introduction
============

This guide gives an overview of using the IIO firmware application with Analog Devices 
AD579x Evaluation board and SDP-K1 (or other compatible) MCU controller board, 
leveraging STM32 as a primary software platform. This firmware application 
leverages the ADI developed IIO (Industrial Input Output) ecosystem to evaluate 
the AD579x (IIO) device by providing device configuration and data streaming support.

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

   .. image:: /source/projects/ad579x_iio/ad579x_interface_diagram.jpg
      :width: 700

===============
Jumper Settings
===============

**SDP-K1:**

Connect the VIO_ADJUST jumper on the SDP-K1 board to 3.3V position to drive SDP-K1 GPIOs at 3.3V

**EVAL-AD5780ARDZ:**

.. note::

   Below jumper settings are the default settings for AD5780 EVB.

+-----------------------+-----------------------+
|Jumper                 | Default Position      | 
+-----------------------+-----------------------+
|LK1                    | Installed             |
+-----------------------+-----------------------+
|LK2                    | 1 and 2 Position      | 
+-----------------------+-----------------------+
|LK3                    | 1 and 2 Position      |
+-----------------------+-----------------------+
|LK4                    | 1 and 2 Position      | 
+-----------------------+-----------------------+

**EVAL-AD5781ARDZ:**

.. note::

   Below jumper settings are the default settings for AD5781 EVB.

+-----------------------+-----------------------+
|Jumper                 | Default Position      | 
+-----------------------+-----------------------+
|LK1                    | Installed             |
+-----------------------+-----------------------+
|LK2                    | 1 and 2 Position      | 
+-----------------------+-----------------------+
|LK3                    | 1 and 2 Position      |
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
