AD552XR IIO Application
""""""""""""""""""""""""

==================
Supported Hardware
==================

**Supported Devices:**

* `AD5529R <https://www.analog.com/en/products/ad5529r.html>`_

**Supported Evaluation Boards:**

* `EVAL-AD5529R <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad5529r.html#eb-overview>`_

**Supported Carrier Boards:**

* `SDP-K1 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/sdp-k1.html>`_

============
Introduction
============

This guide gives an overview of using the IIO firmware application with Analog Devices
AD552XR Evaluation board and SDP-K1 (or other compatible) MCU controller board,
leveraging STM32 as a primary software platform. This firmware application
leverages the ADI developed IIO (Industrial Input Output) ecosystem to evaluate
the AD552XR (IIO) device by providing device configuration and data streaming support.

   .. image:: /source/tinyiiod/app_interface.png
      :width: 350

The interface used for communicating with PC based IIO clients is either Virtual Serial Or UART.
IIO Firmware leverages the ADI created no-os and platform driver software layers
to communicates with IIO device.

.. Useful links Section

.. include:: /source/useful_links.rst

====================
Hardware Connections
====================
Required: SDP-K1 (or alternative STM32 enabled controller board), EVAL-AD5529r board and USB cable.
Connect the EVAL-AD5529r board to SDP-K1 board (or any other STM32 enabled controller board).
Connect SDP-K1 board to the PC using the USB cable.
   .. image:: /source/projects/ad552xr_iio/ad552xr_connection_diagram.png
      :width: 700

===============
Jumper Settings
===============

**SDP-K1:**

Connect the VIO_ADJUST jumper on the SDP-K1 board to 3.3V position to drive SDP-K1 GPIOs at 3.3V

**EVAL-AD5529R:**

Please refer to the user guide for the jumper connections on the EVAL-AD5529R board

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
