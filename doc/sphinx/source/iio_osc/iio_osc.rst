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
called as 'Attributes“. There are 3 types of attributes:

* Device Attributes (Global): Access/Configure common device parameters.
* Channel Attributes (Specific to channels): Access/Configure channel specific device parameters.
* Board Level Attributes (Can be Global and Channel specific): Configure the board level parameters. 
  Projects only receive the board-level attribute when deemed necessary, thus it is not a standard inclusion

   .. image:: /source/iio_osc/iio_osc_attributes.png
      :width: 500

.. warning::
   
   After any board-level attribute is configured, the 'reconfigure_system' 
   attribute must be enabled. This step ensures that the IIO Device is reinitialized and
   the changes made through the selected board-level attributes are applied. 
   If this step is neglected, the changes made on the attributes will not take effect on the device.
   The IIO Oscilloscope application must be closed and re-opened to ensure continued communication with the IIO Device.

Using IIO Oscilloscope for ADCs
-------------------------------

.. Data Capture & DMM:
    
.. include:: data_capture_and_dmm.rst

Using IIO Oscilloscope for DACs
-------------------------------

.. Data Streaming:
    
.. include:: data_streaming.rst