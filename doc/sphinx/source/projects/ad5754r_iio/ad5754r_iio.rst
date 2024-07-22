AD5754R/CN0586 IIO Application
""""""""""""""""""""""""""""""

==================
Supported Hardware
==================

**Supported Devices:**

* `AD5754R <https://www.analog.com/en/products/ad5754r.html>`_ 

**Supported Evaluation Boards:**

* `EVAL-CN0586-ARDZ <https://www.analog.com/en/resources/reference-designs/circuits-from-the-lab/cn0586.html#rd-overview>`_
* `EVAL-AD5754R <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad5754r.html?`_

**Supported Carrier Boards:**

* SDP-K1 With `Mbed Platform <https://os.mbed.com/platforms/SDP_K1/>`_ 

============
Introduction
============

This page gives an overview of using the ARM platforms supported (Mbed) 
firmware example with Analog Devices EVAL-CN0586-ARDZ and EVAL-AD5754R Evaluation boards and SDP-K1 controller board. 
This example code leverages the ADI developed IIO (Industrial Input Output) ecosystem to 
evaluate the AD5754R/CN0586 device by providing a device debug and data capture support.
The code provides support for MBED platform, using the SDP-K1 controller board.

The build guide for the Mbed platform can be found in the further sections.

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

Required: SDP-K1 (or alternative Mbed enabled controller board), EVAL-AD5754R board or EVAL-CN0586-ARDZ board,
and USB cable.

When using the EVAL-CN0586-ARDZ, connect the EVAL-CN0586 board to the controller board by stacking the CN0586's digital interface pins 
onto the controller board's Arduino Uno V3 header.
Connect controller board to the PC using the USB cable. 

   .. image:: /source/projects/ad5754r_iio/ad5754r_interface_diagram.png
      :width: 600

When using the EVAL-AD5754R, make the connections as follows using jumper wires:

+-------------+---------------+
| SDP-K1      |  EVAL-AD5754R |
+-------------+---------------+
| D13         |  J8 Pin 9     |
+-------------+---------------+
| D12         |  J8 Pin 1     |
+-------------+---------------+
| D11         |  J8 Pin 7     |
+-------------+---------------+
| D10         |  J8 Pin 10    |
+-------------+---------------+
| D6          |  J8 Pin 7     |
+-------------+---------------+
| D4          |  J8 Pin 4     |
+-------------+---------------+
| DGND        |  J8 Pin 3     |
+-------------+---------------+

====================================
EVAL-CN0586-ARDZ Evaluation Firmware
====================================

By default, the firmware exposes two IIO Devices: "ad5754r" and "cn0586". The "cn0586" IIO device allows configuring the output's state,
range and voltage level. For a more granular access to the DAC, the "ad5754r" IIO device can be used to configure 
IIO attributes tied to the DAC's bitfields, or even perform direct register reads and writes.

================================
EVAL-AD5754R Evaluation Firmware 
================================

To build the firmware to use with EVAL-AD5754R, comment the "DEV_CN0586" macro definition in app_config.h. When using the 
EVAL-AD5754R, configure the jumpers to allow the SDP-K1 (or any other Mbed enabled controller board) to
control the digital interface. Refer to the Evaluation Board user guide for the jumper configurations.

===============
Jumper Settings
===============

SDP-K1 :

Connect the VIO_ADJUST jumper on the SDP-K1 to 3.3V position 
to drive SDP-K1 GPIOs at 3.3V

**EVAL-AD5754R and EVAL-CN0586-ARDZ:**

Please refer to the user guide for the jumper connections on the Evaluation Boards.

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