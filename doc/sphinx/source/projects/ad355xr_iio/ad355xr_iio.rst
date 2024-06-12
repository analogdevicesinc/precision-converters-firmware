AD355XR IIO Application
""""""""""""""""""""""""

==================
Supported Hardware
==================

**Supported Devices:**

* `AD3541R <https://www.analog.com/en/products/ad3541r.html>`_

* `AD3542R <https://www.analog.com/en/products/ad3542r.html>`_

* `AD3551R <https://www.analog.com/en/products/ad3551r.html>`_

* `AD3552R <https://www.analog.com/en/products/ad3552r.html>`_

**Supported Evaluation Boards:**

* `EVAL-AD3542R <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad3542r.html#eb-overview>`_
* `EVAL-AD3552R <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad3552r.html>`_

**Supported Carrier Boards:**

* `SDP-K1 With Mbed <https://os.mbed.com/platforms/SDP_K1/>`_

============
Introduction
============

This guide gives an overview of using the IIO firmware application with Analog Devices 
AD355XR Evaluation board and SDP-K1 (or other compatible) MCU controller board, 
leveraging Mbed-OS as a primary software platform. This firmware application 
leverages the ADI developed IIO (Industrial Input Output) ecosystem to evaluate 
the AD355XR (IIO) device by providing device configuration and data streaming support.
The code provides support to MBED and STM32 platforms.

The respective build guides for each of the platforms can be found in the further sections. 
The active platform can be chosen by selecting the appropriate value for 
the ACTIVE_PLATFORM macro in the app_config.h (Default is mbed)

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
Required: SDP-K1 (or alternative Mbed enabled controller board), EVAL-AD355xr board and USB cable.
Connect the EVAL-AD355xr board to SDP-K1 board (or any other Mbed enabled controller board). 
Connect SDP-K1 board to the PC using the USB cable.
   .. image:: /source/projects/ad355xr_iio/ad355xr_connection_diagram.png
      :width: 700

===============
Jumper Settings
===============

**SDP-K1:**

Connect the VIO_ADJUST jumper on the SDP-K1 board to 3.3V position to drive SDP-K1 GPIOs at 3.3V

**EVAL-AD355XR:**

Please refer to the user guide for the jumper connections on the EVAL-AD355XR board

.. Communication Interface section:

.. include:: /source/hardware/comm_interface.rst

.. Project Build Section:
    
.. include:: /source/build/project_build.rst

.. IIO Ecosystem Section:
    
.. include:: /source/tinyiiod/iio_ecosystem.rst

.. note::

    While sending DAC codes from any of IIO Clients to DAC, provide 1st channel data first 
    then 0th channel data.

.. IIO Firmware Structure

.. include:: /source/tinyiiod/iio_firmware_structure.rst

=======
Support
=======

Feel free to ask questions in the `EngineerZone <https://ez.analog.com/data_converters>`_
