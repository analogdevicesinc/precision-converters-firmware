AD2S1210 IIO Application
""""""""""""""""""""""""

==================
Supported Hardware
==================

**Supported Devices:**

* `AD2S1210 <https://www.analog.com/en/products/ad2s1210.html>`_

**Supported Evaluation Boards:**

* `EVAL-AD2S1210 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/ad2s1210.html>`_

**Supported Carrier Boards:**

* `SDP-K1 With Mbed <https://os.mbed.com/platforms/SDP_K1/>`_

============
Introduction
============

This page gives an overview of using the ARM platforms supported (default is Mbed) 
firmware example with Analog Devices AD2S1210 Evaluation board and SDP-K1 controller board. 
This example code leverages the ADI developed IIO (Industrial Input Output) ecosystem to 
evaluate the AD2S1210 device by providing a device debug and data capture support.

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

Required: SDP-K1, ADZS-BRKOUT, EVAL-AD2S1210 board and USB cable.
Plug in the EVAL-AD2S1210 board to the ADZS-BRKOUT, populate flywires to the
SDP-K1 board using the Arduino connector.

The connections are as follows:
+-------------+------------+
| SDPK1	      |  BREAKOUT  |
+-------------+------------+
| DIGITAL-0   | 96  A0     |
+-------------+------------+
| DIGITAL-1   | 25  A1     |
+-------------+------------+
| DIGITAL-4   | 48  SAMPLE |
+-------------+------------+
| DIGITAL-5   | 43  RES0   |
+-------------+------------+
| DIGITAL-6   | 78  RES1   |
+-------------+------------+
| DIGITAL-10  | 100 nWD    |
+-------------+------------+
| DIGITAL-11  | 12  SDI    |
+-------------+------------+
| DIGITAL-12  | 110 SDO    |
+-------------+------------+
| DIGITAL-13  | 13  SCLK   |
+-------------+------------+
| POWER-GND   | 4 GIO      |
+-------------+------------+
| POWER-3v3   | 116 VIO    |
+-------------+------------+

ad2S1210-EVAL
J3 CS - J3 GND

Connect the SDP-K1 board to the PC using the USB cable and the AD2S1210 EVB to the
provided AC adapter.

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
