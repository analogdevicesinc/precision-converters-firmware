AD7124 Temperature Measure Firmware Application
"""""""""""""""""""""""""""""""""""""""""""""""

==================
Supported Hardware
==================

**Supported Devices:**

* `AD7124-8 <https://www.analog.com/en/products/ad7124-8.html>`_

* `AD7124-4 <https://www.analog.com/en/products/ad7124-4.html>`_

**Supported Evaluation Boards:**

* `EVAL-AD7124-8 <https://www.analog.com/en/design-center/evaluation-hardware-and-software/evaluation-boards-kits/EVAL-AD7124-8.html>`_

* `EVAL-AD7124-4 <https://www.analog.com/en/design-center/evaluation-hardware-and-software/evaluation-boards-kits/EVAL-AD7124-4.html>`_

**Supported Carrier Boards:**

* `SDP-K1 With Mbed <https://os.mbed.com/platforms/SDP_K1/>`_

============
Introduction
============

This guide gives an overview of using the temperature measurement console firmware 
application with Analog Devices AD7124 Evaluation board and SDP-K1 (or other compatible)
MCU controller board, leveraging Mbed-OS as a primary software platform. 
The firmware application provides an user-interactive menu options for user to select 
and configure the multiple temperature sensors at different instances, such as 
2/3/4-wire RTDs, NTC Thermistors and Thermocouples.

   .. image:: /source/projects/ad7124_temperature-measure/ad7124_interface.png
      :width: 600

The interface used for communicating with PC based console tools (e.g. Teraterm) is UART.
Firmware leverages the ADI created no-os and platform driver software layers
to communicates with ADC device.

.. SDP-K1 Mbed Section

.. include:: /source/tinyiiod/sdp_k1_mbed.rst

.. Useful links Section

.. include:: /source/useful_links.rst

====================
Hardware Connections
====================

   .. image:: /source/projects/ad7124_temperature-measure/ad7124_hardware_connections.png
      :width: 800

===============
Jumper Settings
===============

**SDP-K1:**

Connect the VIO_ADJUST jumper on the SDP-K1 board to 3.3V position to drive SDP-K1 GPIOs at 3.3V

**EVAL-AD7124:**

Refer user manual of required evaluation board.

=======================
Communication Interface
=======================

SDP-K1 is powered through USB connection from the computer. SDP-K1 MCU board acts 
as a serial device when connected to PC, which creates a serial ports to connect 
to console based tool running on PC. The serial port assigned to a device can 
be seen through the device manager for windows-based OS as shown below. Use UART
(physical) port for PC communication.

   .. image:: /source/hardware/serial_ports_view.png
      :width: 400


.. Project Build Section:
    
.. include:: /source/build/project_build.rst

==============
Using Firmware
==============

Configure your serial terminal ( Tera Term) for below settings:

   .. image:: /source/console/console_app_teraterm_settings.png
      :width: 600

The AD7124 temperature measurement example main menu looks like below (with Tera Term):

   .. image:: /source/projects/ad7124_temperature-measure/ad7124_temperature_measure_main_menu.png
      :width: 600

Firmware allows user to perform the measurement for single or multiple (more than one) 
temperature sensors of same type. Below sensors are supported:

* Single or Multiple 2/3/4-wire RTDs (default is PT100)

* Single Or Multiple Thermocouple (default is T-type)

* Single Or Multiple Thermistors (default is 10K NTC)

User must ensure all sensors are connected to AD7124 evaluation board as per 
configurations specific in the software and on this wiki page (see subsequent 
sections). If user intend to change these configurations in the software, the 
hardware connections must be modified as per new configurations. The details 
about altering the software modules for modifying the configurations are given 
in 'Modifying Firmware' section.

   .. image:: /source/projects/ad7124_temperature-measure/bypassing_lk6_link_on_legacy_board.png
      :width: 600

.. note::

   In order to use analog inputs AIN4 and AIN5 on Legacy AD7124 Eval board 
   (with SDP-120 interface only) in any of the demo mode, make sure to route the 
   sensor connections directly through LK6 link, instead of physical screw-terminal 
   connector. Make sure LK6 link is removed for the same purpose.

========================================
Multiple RTD (2/3/4-wire Configurations)
========================================

Reference: https://www.analog.com/en/design-center/reference-designs/circuits-from-the-lab/CN0383.html

**Multiple 2-wire RTD configurations:**

   .. image:: /source/projects/ad7124_temperature-measure/ad7124_multiple_2_wire_rtd_configs.png
      :width: 800

**Multiple 3-wire RTD configurations:**

   .. image:: /source/projects/ad7124_temperature-measure/ad7124_multiple_3_wire_rtd_configs.png
      :width: 800

**Multiple 4-wire RTD configurations:**

   .. image:: /source/projects/ad7124_temperature-measure/ad7124_multiple_4_wire_rtd_configs.png
      :width: 800

====================================
Multiple Thermocouple Configurations
====================================

Reference: https://www.analog.com/en/design-center/reference-designs/circuits-from-the-lab/CN0384.html

**Thermocouple (Hot Junction) configurations for TC measurement:**

   .. image:: /source/projects/ad7124_temperature-measure/ad7124_multiple_tc_configs.png
      :width: 800

**Cold Junction Compensation (CJC) configurations for TC measurement:**

   .. image:: /source/projects/ad7124_temperature-measure/ad7124_cjc_configs.png
      :width: 800

======================================
Multiple NTC Thermistor Configurations
======================================

Reference: https://www.analog.com/en/design-center/reference-designs/circuits-from-the-lab/cn0545.html

   .. image:: /source/projects/ad7124_temperature-measure/ad7124_multiple_ntc10k_configs.png
      :width: 800

=======================
Calibrating 3-wire RTDs
=======================

Firmware provides an option for user to perform calibrated measurement on 3-wire 
RTD sensors. There are two types of calibration options available in the firmware. 
User must modify sensor hardware connections according to configuration defined 
in the software. Based on selected calibration type, the sensors are first calibrated 
and then 3-wire RTD measurement is performed on them. For more information on the 
calibration scheme, refer the `design note <https://www.analog.com/en/design-center/reference-designs/circuits-from-the-lab/CN0383.html>`_ 
on RTD measurement.

   .. image:: /source/projects/ad7124_temperature-measure/ad7124_3wire_rtd_calibration_menu.png
      :width: 800

.. note::

   Calibration is allowed to perform only on Multiple RTD sensors and not on a single RTD sensor.

=================================================
Calibrating ADC (Internal and System Calibration)
=================================================

Firmware allows user to perform ADC calibration on the current sensor configuration 
(demo mode) selected through console menu. ADC calibration helps to remove any offset 
Or gain error present on the input channels. The updated device coefficients (gain and offset) 
post ADC calibration are used/applied during the sensor measurement. Therefore, 
once calibration is complete, user must go to the previous demo mode which was 
selected before ADC calibration and then perform the measurement on selected sensors. 
The calibration coefficients (gain and offset) are applied only on the analog input 
channels which were enabled prior to calibration. Also after calibration if any 
new demo mode is selected apart from the one which was enabled during calibration, 
the calibration coefficients are reset and doesn't applied on input channels. In 
this case, user must perform the calibration again.

Internal ADC calibration is straightforward but system calibration needs user 
inputs (typically after applying full-scale/zero-scale) voltages on selected 
analog inputs.

   .. image:: /source/projects/ad7124_temperature-measure/ad7124_calibration_menu.png
      :width: 800


==================
Firmware Structure
==================

   .. image:: /source/tinyiiod/firmware_architecture.png
      :width: 600

=======
Support
=======

Feel free to ask questions in the `EngineerZone <https://ez.analog.com/data_converters>`_
