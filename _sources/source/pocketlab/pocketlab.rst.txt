============
Introduction
============
Pocket lab is a portable display-based embedded solution created primarily for demoing the ADCs functionality.
Pocket Lab makes use of `STM32F769NI-Discovery board <https://www.st.com/en/microcontrollers-microprocessors/stm32f769ni.html>`_ from ST microelectronics and 3rd party library LVGL.
The demo functionality was primarily focused on device configuration, register memory access, data capture and FFT analysis.
This documentation describes various components of pocket lab application in subsequent sections.

.. Pocketlab Firmware Structure
.. include:: /source/pocketlab/pocketlab_firmware_structure.rst

=================
Pocketlab Library
=================

Source Code: `Precision Converters Library <https://github.com/analogdevicesinc/precision-converters-library/tree/main/pocket_lab>`_
Doxygen: `Doxygen for Pocket Lab <https://analogdevicesinc.github.io/precision-converters-firmware/doxygen/dir_e754d0f20ca3746dccad9be70ecb80f9.html>`_

Pocket lab library makes use of 'lvgl (Light versatile graphics library)'. LVGL is primarily targeted for the embedded display-based applications. LVGL is an abstracted library, which supports multiple display boards from the different vendors. So, it's vendors responsibility to add lvgl support for their display board. 
This version of pocket lab library is based on the latest development version of lvgl library. The source code link is: https://github.com/lvgl/lvgl/tree/7506b61527195fdc96803fe94f5ba141b6e5a9e8

LVGL uses vendor created portable library to handle the display and touchscreen events. Current version of pocket lab library is targeted for the STM32F769NI Discovery board from ST microelectronics. 
The lvgl portable library for the board used is present here: https://github.com/lvgl/lv_port_stm32f769_disco/tree/4f26b19d42816f10fb8e51a968b2ec924c1e75ba

Pocket Lab library mainly contains 3 source modules:

* pl_gui_views.c/.h
* pl_gui_iio_wrappers.c/.h
* pl_gui_events.c/.h

===================
Running Pocket Lab
===================

Hardware setup
--------------
Stack the Eval board on the discovery board. Power the board with USB to Micro B cable through the USB ST link port on the board as shown below

* Comment out the pocketlab libraries to include them in .mbedignore.
* Change the ACTIVE_IIO_CLIENT to IIO_CLIENT_REMOTE to select pocketlab as a client.

.. image:: /source/pocketlab/mbedignore_modifications.png
   :width: 500

Flash the binary on the board and the board gets powered on with a default view as shown below

.. image:: /source/pocketlab/def_view.jpg
   :width: 500

.. Pocketlab GUI Views
.. include:: /source/pocketlab/pl_gui_views.rst

.. note::
    Make sure to select the channels and then press the start button for capture / dmm view. Only one PL GUI view should be accessed at a time.
    If the display misbehaves, press the onboard reset or replug the board. 
