IIO Oscilloscope
----------------

============
Installation
============

Refer this page to install IIO oscilloscope for Windows or Linux OS: `IIO Oscilloscope (Client) <https://github.com/analogdevicesinc/iio-oscilloscope/releases>`_

================
Running on Linux
================

Refer this page: `<https://wiki.analog.com/resources/tools-software/linux-software/iio_oscilloscope>`_

==================
Running on Windows
==================

Open the IIO Oscilloscope application from start menu and configure the serial (UART) 
settings as shown below. Click on refresh button and device should pop-up 
in the IIO devices list.

   .. image:: /source/iio_osc/iio_osc_connect_window.png
      :width: 300

Click 'Connect' and it should by default open the data ‘Capture’ window. You can
drag aside or close this window to see the main ‘Debug and DMM’ tab window.

   .. image:: /source/iio_osc/iio_osc_tabs.png
      :width: 500

===============================================
Configure/Access Device Attributes (Parameters)
===============================================

The IIO Oscilloscope allows user to access and configure different device parameters, 
called as 'Attributes“. There are 2 types of attributes:

* Device Attributes (Global): Access/Configure common device parameters.
* Channel Attributes (Specific to channels): Access/Configure channel specific device parameters.

Using IIO Oscilloscope for ADCs
-------------------------------

.. Data Capture & DMM:
    
.. include:: data_capture_and_dmm.rst

Using IIO Oscilloscope for DACs
-------------------------------

.. Data Streaming:
    
.. include:: data_streaming.rst