Digipots IIO Application
""""""""""""""""""""""

==================
Supported Hardware
==================

**Supported Devices:**

* `AD5141 <https://www.analog.com/media/en/technical-documentation/data-sheets/AD5121_5141.pdf>`_
* `AD5142 <https://www.analog.com/media/en/technical-documentation/data-sheets/ad5122_5142.pdf>`_
* `AD5142A <https://www.analog.com/media/en/technical-documentation/data-sheets/ad5122a_5142a.pdf>`_
* `AD5143 <https://www.analog.com/media/en/technical-documentation/data-sheets/ad5123_5143.pdf>`_
* `AD5144 <https://www.analog.com/media/en/technical-documentation/data-sheets/ad5124_5144_5144a.pdf>`_
* `AD5160 <https://www.analog.com/media/en/technical-documentation/data-sheets/ad5160.pdf>`_
* `AD5161 <https://www.analog.com/media/en/technical-documentation/data-sheets/AD5161.pdf>`_
* `AD5165 <https://www.analog.com/media/en/technical-documentation/data-sheets/AD5165.pdf>`_
* `AD5171 <https://www.analog.com/media/en/technical-documentation/data-sheets/AD5171.pdf>`_
* `AD5241 <https://www.analog.com/media/en/technical-documentation/data-sheets/ad5241_ad5242.pdf>`_
* `AD5242 <https://www.analog.com/media/en/technical-documentation/data-sheets/ad5241_ad5242.pdf>`_
* `AD5245 <https://www.analog.com/media/en/technical-documentation/data-sheets/ad5245.pdf>`_
* `AD5246 <https://www.analog.com/media/en/technical-documentation/data-sheets/AD5246.pdf>`_
* `AD5258 <https://www.analog.com/media/en/technical-documentation/data-sheets/AD5258.pdf>`_
* `AD5259 <https://www.analog.com/media/en/technical-documentation/data-sheets/ad5259.pdf>`_
* `AD5273 <https://www.analog.com/media/en/technical-documentation/data-sheets/ad5273.pdf>`_

**Supported Evaluation Boards:**

* `EVAL-AD5141 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad5141.html>`_
* `EVAL-AD5142 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad5142.html>`_
* `EVAL-AD5142A <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad5142a.html>`_
* `EVAL-AD5143 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad5143.html>`_
* `EVAL-AD5144 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad5144.html>`_
* `EVAL-AD5160 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad5160.html>`_
* `EVAL-AD5161 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad5161.html>`_
* `EVAL-AD5165 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad5165.html>`_
* `EVAL-AD5171 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad5171.html>`_
* `EVAL-AD5242 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad5242.html>`_
* `EVAL-AD5245 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad5245.html>`_
* `EVAL-AD5246 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad5246.html>`_
* `EVAL-AD5258 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad5258.html>`_
* `EVAL-AD5259 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad5259.html>`_
* `EVAL-AD5273 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad5273.html>`_

**Evaluation Mother Board:**

* EVAL-MB-LV-ARDZ

**Supported Carrier Boards:**

* `SDP-K1  <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/sdp-k1.html#eb-overview>`_

============
Introduction
============

This page gives an overview of using the ARM platforms supported
firmware example with Analog Devices EVAL-MB-LV-ARDZ board and SDP-K1 controller board.
This example code leverages the ADI developed IIO (Industrial Input Output) ecosystem to
evaluate the digital potentiometer device by providing a device configuration support.

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

Required: SDP-K1 (or an STM32 board), any supported DigiPOT evaluation board, EVAL-MB-LV-ARDZ motherboard and USB cable.

Connect the EVAL-MB-LV-ARDZ motherboard with the evaluation board to SDP-K1 board (or an equivalent STM32 board). Connect controller board to the PC using the USB cable.

   .. image:: /source/projects/digipots_iio/digipots_connection_diagram.png
      :width: 600

===============
Jumper Settings
===============

**SDP-K1:**

Connect the VIO_ADJUST jumper on the SDP-K1 to 3.3V position to drive SDP-K1 GPIOs at 3.3V.

**EVAL-MB-LV-ARDZ:**

Please refer to the user guide for the jumper connections on the EVAL-MB-LV-ARDZ board

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
