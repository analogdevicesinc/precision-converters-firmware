AD7124 IIO Application
""""""""""""""""""""""

==================
Supported Hardware
==================

**Supported Devices:**

* `AD7124-4 <https://www.analog.com/en/products/ad7124-4.html>`_
* `AD7124-8 <https://www.analog.com/en/products/ad7124-8.html>`_

**Supported Evaluation Boards:**

* `EVAL-AD7124-4 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad7124-4.html>`_
* `EVAL-AD7124-8 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad7124-8.html>`_

**Supported Carrier Boards:**

* `SDP-K1 With Mbed <https://os.mbed.com/platforms/SDP_K1/>`_
* `Nucleo-H563ZI With STM32 Platform <https://www.st.com/en/evaluation-tools/nucleo-h563zi.html>`_

============
Introduction
============

This page gives an overview of using the ARM platforms supported (default is Mbed) 
firmware example with Analog Devices AD7124 Evaluation board and SDP-K1 controller board. 
This example code leverages the ADI developed IIO (Industrial Input Output) ecosystem to 
evaluate the AD7124 device by providing a device debug and data capture support.

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

Required: SDP-K1 (or alternative Mbed enabled controller board), EVAL-AD7124 board
and USB cable.

Connect the EVAL-AD7124 board to SDP-K1 board (or any other Mbed enabled controller 
board) using jumper wires. Connect SDP-K1 board to the PC using the USB cable. 

   .. image:: /source/projects/ad7124_iio/ad7124_hardware_connection.png
      :width: 700

===============
Jumper Settings
===============

**SDP-K1 / Nucleo-H563ZI :**

Connect the VIO_ADJUST jumper on the SDP-K1 / JP4 on Nucleo-H563ZI board to 3.3V position to drive SDP-K1 / Nucleo-H563ZI GPIOs at 3.3V

**EVAL-AD7124:**

* Stack the EVAL-AD7124-ASDZ on the Arduino connectors of the SDP-K1 board / Nucleo-H563ZI.
* Connect a male-to-male jumper wire between D8 and D12 on Arduino connectors. (Required for data capture)
* If using analog inputs AIN0/AIN1, remove the noise test LK1 jumper.

.. note::
   Set capture_timeout of iio oscilloscope atleast 25s to avoid timing out of the application when capturing all channels at lower odr. 
   Please refer to the limitations section of ADC Data Capture in IIO Tools section

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
