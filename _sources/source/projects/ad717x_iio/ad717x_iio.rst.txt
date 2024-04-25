AD717x / AD411x IIO Application
"""""""""""""""""""""""""""""""

==================
Supported Hardware
==================

**Supported Devices:**

* `AD4111 <https://www.analog.com/en/products/ad4111.html>`_ `AD4112 <https://www.analog.com/en/products/ad4112.html>`_ `AD4114 <https://www.analog.com/en/products/ad4114.html>`_ `AD4115 <https://www.analog.com/en/products/ad4115.html>`_ `AD4116 <https://www.analog.com/en/products/ad4116.html>`_
* `AD7172-2 <https://www.analog.com/en/products/ad7172.html>`_  `AD7172-4 <https://www.analog.com/en/products/ad7172-4.html>`_
* `AD7173-8 <https://www.analog.com/en/products/ad7173-8.html>`_ `AD7175-2 <https://www.analog.com/en/products/ad7175-2.html>`_ `AD7175-8 <https://www.analog.com/en/products/ad7175-8.html>`_
* `AD7176-2 <https://www.analog.com/en/products/ad7176-2.html>`_
* `AD7177-2 <https://www.analog.com/en/products/ad7177-2.html>`_

**Supported Evaluation Boards:**

* `EVAL-AD4111 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad4111.html>`_ `EVAL-AD4112 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad4112.html>`_ `EVAL-AD4114 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad4114.html>`_ `EVAL-AD4115 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad4115.html>`_ `EVAL-AD4116 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad4116.html>`_
* `EVAL-AD7172-2 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad7172-2.html>`_  `EVAL-AD7172-4 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad7172-4.html>`_
* `EVAL-AD7173-8 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad7173-8sdz.html>`_ `EVAL-AD7175-2 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad7175-2.html>`_ `EVAL-AD7175-8 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad7175-8.html>`_
* `EVAL-AD7176-2 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad7176-2.html>`_
* `EVAL-AD7177-2 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad7177-2.html>`_

**Supported Carrier Boards:**

* `SDP-K1 With Mbed <https://os.mbed.com/platforms/SDP_K1/>`_

============
Introduction
============

This page gives an overview of using the ARM platforms supported (default is Mbed) 
firmware example with Analog Devices AD717x Evaluation board and SDP-K1 controller board. 
This example code leverages the ADI developed IIO (Industrial Input Output) ecosystem to 
evaluate the AD717x/AD411x device by providing a device debug and data capture support.

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

Required: SDP-K1 (or alternative Mbed enabled controller board), EVAL-AD717x board
and USB cable.

Connect the EVAL-AD717x board to SDP-K1 board (or any other Mbed enabled controller 
board) using jumper wires. Connect SDP-K1 board to the PC using the USB cable. 


   .. image:: /source/projects/ad717x_iio/ad717x_hardware_interface.png
      :width: 600


===============
Jumper Settings
===============

**SDP-K1:**

Connect the VIO_ADJUST jumper on the SDP-K1 board to 3.3V position to drive SDP-K1 GPIOs at 3.3V

**EVAL-AD717X:**

* Please refer to the respective board user guide on the product page of the chosen device.

.. note::
   In order to capture signals from the AD717x/AD411x board using continuous data capturing, there needs to be an external connection from the MISO pin on the board to the D8 of the arduino header.
   Example- in case of the AD4111, the J10-3 needs to be connected to D8 on the SDP-K1.

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
