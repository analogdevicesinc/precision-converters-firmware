====================
Pocket Lab GUI Views
====================
This module takes care of any event happening on the display GUI. The GUI is created in the form of widgets such as buttons, text fields, drop-down menus, etc. Each widget would have associated event handler callback function, that would be registered for a given event object during pocket lab GUI initialization time. 
The event handler function would be responsible for handling the user event and forming a proper response to that event.
All the widgets, event objects and event callbacks are created using the lvgl library. 
Below are the standard views available for pocket lab and are created from the pocket lab library. 
Each Tinyiiod firmware application provides an ability to enable/disable these views.

Configuration View
------------------
Provides ability to read and write the global and channel attributes of the device.

.. image:: /source/pocketlab/config_view.jpg
   :width: 400

Register View
-------------
This view provides low level access to the registers of the device. The user can read the register values in hex.

.. image:: /source/pocketlab/register_view.jpg
   :width: 400

DMM View
--------
This view continuously displays device specific ADC read back value in SI Units, once start button is activated.
There is an option to enable and disable all the channels.

.. image:: /source/pocketlab/dmm_view.jpg
   :width: 400

Capture View
------------
In this view adc raw data can be captured and visualized as waveforms continuously once the start button is activated.

.. image:: /source/pocketlab/capture_view.jpg
   :width: 400

Analysis View
-------------
In this view FFT analysis can be visualized. Key information such as SNR, THD, Fund Power, freq and RMS noise is displayed on the right pane.

.. note::
FFT analysis can be performed only on one channel at a time.

.. image:: /source/pocketlab/fft_view.jpg
   :width: 400

About View
----------
About screen is used to provide high level product features and some presentation material on the product's value and USP

.. image:: /source/pocketlab/about_view.jpg
   :width: 400
