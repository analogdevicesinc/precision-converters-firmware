Using IIO Oscilloscope
----------------------

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

==================================================
Using DMM Tab to Read DC Voltage on Input Channels
==================================================

DMM tab can be used read the instantaneous voltage applied on analog input channels. 
Simply select the device and channels to read and press start button.

.. note::
   
   The voltage is just instantaneous, so it is not possible to get RMS AC voltage
   or averaged DC voltage. Also, when using DMM tab, do not access/use the Data 
   capture or Debug tab as this could impact data capturing. All uses same 
   communication bus to access the data which could result into access/busy conflicts
   during data capture.

.. image:: /source/iio_osc/iio_osc_dmm_tab.png
   :width: 500

============================
Data Capture from IIO Device
============================

To capture the data from IIO device, simply select the device and channels
to read/capture data. The data is plotted as “ADC Raw Value” Vs “Number of Samples” 
and is just used for Visualization. The data is read as is from device without 
any processing. If user wants to process the data, it must be done externally 
by capturing data from the Serial link on controller board.

.. note::
   
   The DMM or Debug tab should not be accessed when capturing data as this would 
   impact data capturing. All uses same communication bus to access the data which 
   could result into access/busy conflicts during data capture.

*Time Domain plot:*

   .. image:: /source/iio_osc/iio_osc_time_domain_plot.png
      :width: 800

*Frequency Domain plot:*

   .. image:: /source/iio_osc/iio_osc_freq_domain_plot.png
      :width: 800

.. note::

      Select number of samples and channels according to sampling rate of your
      device. For very slow ODRs, the data capturing would be too slow and 
      IIO oscilloscope might become unresponsive waiting for data to be received
      from IIO device.
