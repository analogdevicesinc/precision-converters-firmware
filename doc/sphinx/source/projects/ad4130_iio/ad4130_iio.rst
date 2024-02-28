AD4130 IIO Application
----------------------

==================
Supported Hardware
==================

**Supported Devices:**

* `AD4130-8 <https://www.analog.com/en/products/ad4130-8.html>`_

**Supported Evaluation Boards:**

* `EVAL-AD4130-8WARDZ <https://www.analog.com/en/design-center/evaluation-hardware-and-software/evaluation-boards-kits/EVAL-AD4130-8.html>`_

**Supported Carrier Boards:**

* `SDP-K1 With Mbed <https://os.mbed.com/platforms/SDP_K1/>`_

============
Introduction
============

This guide gives an overview of using the IIO firmware application with Analog Devices 
AD4130 Evaluation board and SDP-K1 (or other compatible) MCU controller board, 
leveraging Mbed-OS as a primary software platform. This firmware application 
leverages the ADI developed IIO (Industrial Input Output) ecosystem to evaluate 
the AD4130 (IIO) device by providing device configuration and data capture support.

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

   .. image:: /source/projects/ad4130_iio/ad4130_hardware_connections.png
      :width: 600

===============
Jumper Settings
===============

**SDP-K1:**

Connect the VIO_ADJUST jumper on the SDP-K1 board to 3.3V position to drive SDP-K1 GPIOs at 3.3V

**EVAL-AD4130 (Rev D):**

.. note::

   Below jumper settings of AD4130 EVB are specific to sensor demo modes supported in the firmware. Change the
   jumper settings according to your requirements/demo configurations.

+-----------------------+-----------------------+----------------+------------+----------------+----------+-----------------+
|Jumper                 | User Default Config   | Thermistor     | RTD        | Thermocouple   | ECG      | Power Test      |
+-----------------------+-----------------------+----------------+------------+----------------+----------+-----------------+
|LK1, LK2, LK3          | Short                 | Short          | Short      | Short          | Short    | Short           |
+-----------------------+-----------------------+----------------+------------+----------------+----------+-----------------+
|LK4                    | A                     | A              | Open       | Open           | Open     | A               |
+-----------------------+-----------------------+----------------+------------+----------------+----------+-----------------+
|LK5                    | Open                  | Open           | Open       | Open           | Open     | Open            |
+-----------------------+-----------------------+----------------+------------+----------------+----------+-----------------+
|LK6                    | Short                 | Short          | Open       | Open           | Open     | Open            |
+-----------------------+-----------------------+----------------+------------+----------------+----------+-----------------+
|LK7                    | A                     | A              | A          | A              | A        | A               |
+-----------------------+-----------------------+----------------+------------+----------------+----------+-----------------+
|LK8, LK9, LK10, LK11   | Open                  | Open           | Open       | Open           | Open     | Short           |
+-----------------------+-----------------------+----------------+------------+----------------+----------+-----------------+
|LK12                   | Open                  | Open           | Open       | Open           | Open     | Open            |
+-----------------------+-----------------------+----------------+------------+----------------+----------+-----------------+
|LK14                   | Open                  | Open           | Open       | Open           | Open     | Open            |
+-----------------------+-----------------------+----------------+------------+----------------+----------+-----------------+
|LK15, LK16             | A                     | A              | A          | A              | A        | A               |
+-----------------------+-----------------------+----------------+------------+----------------+----------+-----------------+
|LK17, LK18, LK19, LK20 | A                     | A              | A          | A              | A        | A               |
+-----------------------+-----------------------+----------------+------------+----------------+----------+-----------------+
|LK21                   | Open                  | Open           | Open       | Open           | Open     | Open            |
+-----------------------+-----------------------+----------------+------------+----------------+----------+-----------------+
|LK22                   | Open                  | Open           | Open       | Short          | Short    | Open            |
+-----------------------+-----------------------+----------------+------------+----------------+----------+-----------------+
|LK23, LK24             | Open                  | Open           | Open       | Open           | Open     | Open            |
+-----------------------+-----------------------+----------------+------------+----------------+----------+-----------------+
|LK25                   | Open                  | Open           | Open       | Open           | Open     | Open            |
+-----------------------+-----------------------+----------------+------------+----------------+----------+-----------------+
|LK26                   | Short                 | Short          | Short      | Short          | Short    | Short           |
+-----------------------+-----------------------+----------------+------------+----------------+----------+-----------------+
|LK27                   | B                     | B              | B          | B              | B        | B               |
+-----------------------+-----------------------+----------------+------------+----------------+----------+-----------------+
|LK28                   | B                     | B              | B          | B              | B        | B               |
+-----------------------+-----------------------+----------------+------------+----------------+----------+-----------------+
|LK29                   | Short                 | Short          | Short      | Short          | Short    | Short           |
+-----------------------+-----------------------+----------------+------------+----------------+----------+-----------------+
|LK30, LK31, LK32       | Open                  | Open           | Open       | Open           | Open     | Open            |
+-----------------------+-----------------------+----------------+------------+----------------+----------+-----------------+
|LK35                   | Open                  | Open           | Short      | Open           | Open     | Open            |
+-----------------------+-----------------------+----------------+------------+----------------+----------+-----------------+
|LK36                   | Open                  | Open           | Short      | Short          | Short    | Open            |
+-----------------------+-----------------------+----------------+------------+----------------+----------+-----------------+
|LK37                   | Open                  | Short          | Open       | Open           | Open     | Open            |
+-----------------------+-----------------------+----------------+------------+----------------+----------+-----------------+
|LK45                   | Open                  | Open           | Open       | Open           | Open     | Open            |
+-----------------------+-----------------------+----------------+------------+----------------+----------+-----------------+
|J4                     | Open                  | NA             | NA         | NA             | NA       | NA              |
+-----------------------+-----------------------+----------------+------------+----------------+----------+-----------------+
|J5                     | Open                  | NA             | NA         | NA             | NA       | NA              |
+-----------------------+-----------------------+----------------+------------+----------------+----------+-----------------+
|AVDD SW                | 3.3v POS              | 3.3v POS       | 3.3v POS   | 3.3v POS       | NA       | 3.3v POS        |
+-----------------------+-----------------------+----------------+------------+----------------+----------+-----------------+
|IOVDD SW               | 3.3v POS              | 3.3v POS       | 3.3v POS   | 3.3v POS       | NA       | 3.3v POS        |
+-----------------------+-----------------------+----------------+------------+----------------+----------+-----------------+
|REFIN+/- SW            | REFIN1+/-             | REFIN1+/-      | REFIN1+/-  | REFIN1+/-      | NA       | REFIN1+/-       |
+-----------------------+-----------------------+----------------+------------+----------------+----------+-----------------+
|T_AVDD                 | A                     | A              | A          | A              | NA       | A               |
+-----------------------+-----------------------+----------------+------------+----------------+----------+-----------------+
|T_IOVDD                | A                     | A              | A          | A              | NA       | A               |
+-----------------------+-----------------------+----------------+------------+----------------+----------+-----------------+

.. Communication Interface section:

.. include:: /source/hardware/comm_interface.rst

.. Project Build Section:
    
.. include:: /source/build/project_build.rst

.. IIO Ecosystem Section:
    
.. include:: /source/tinyiiod/iio_ecosystem.rst

========================
AD4130 Sensor Demo Modes
========================

AD4130 IIO firmware provides support for interfacing different sensors to analog
inputs and perform the measurement on them. Below sensor demo modes are supported in the firmware.

* User Default Config (All channels test)
* 2-wire/3-wire/4-wire RTD (Default is PT100)
* Thermistor (Default is 10K NTC)
* Thermocouple (Default is T type TC and PT1000 RTD as CJC)
* Load Cell (4-wire bridge)
* Noise Test (AIN0-AIN1 shorted)
* ECG
* Power Test (ADC internal channels voltage/current measurement)

.. note::

   The demo mode configurations can be changed from respective user_config
   header files and some sensor specific parameters can be updated from
   ad4130_temperature_sensor.cpp file.

===================
Demo Mode Selection
===================

The sensor mode selection is done from “app_config.h” file using “ACTIVE_DEMO_MODE_CONFIG”
macro. The selection is done at compilation time, that means only one sensor demo
mode is active at a time. Whenever demo mode is changed from app_config.h file,
the code must be compiled again to generate a new binary file for that.

============================
Demo Mode User Configuration
============================

Firmware maintains the unique user configuration file for each sensor demo mode
as per below table. The configurations can be updated by using .c and .h user config files.

+-----------------------------+---------------------------------+
|Demo Mode                    | User Config Files               |
+-----------------------------+---------------------------------+
| 2-wire/3-wire/4-wire RTD    | ad4130_rtd_config.c/.h          |
+-----------------------------+---------------------------------+
| Thermistor                  | ad4130_thermistor_config.c/.h   |
+-----------------------------+---------------------------------+
| Thermocouple                | ad4130_thermocouple_config.c/.h |
+-----------------------------+---------------------------------+
| Load Cell                   | ad4130_loadcell_config.c/.h     |
+-----------------------------+---------------------------------+
| Noise Test                  | ad4130_noise_test_config.c/.h   |
+-----------------------------+---------------------------------+
| ECG                         | ad4130_ecg_config.c/.h          |
+-----------------------------+----------+----------+-----------+
| Power Test                  | ad4130_power_test_config.c/.h   |
+-----------------------------+---------------------------------+

==================
Sensor Measurement
==================

Sensor measurement for RTD, Thermistor, Thermocouple, Noise Test, ECG, Power Test
and User Default Config can be done using the IIO oscilloscope GUI client application
or by executing python scripts from the ‘scripts’ folder. Temperature result for RTD,
TC and Thermistor would be in degree Celsius. The result for other configs would in voltage/current.

Refer to this guide here to setup IIO enviornment for sensor measurements if not already done:
:doc:`Setting-up IIO Ecosystem </source/tinyiiod/iio_ecosystem>`

Sensor measurement for Load Cell can be done by executing the python script available
in the project “scripts” folder. IIO oscilloscope can only support measurement for voltage,
current and temperature quantities and therefore python code is developed to support
measurement for other sensor types.

.. code-block:: bash

      $ python ad4130_sensor_measurement.py

.. image:: /source/projects/ad4130_iio/ad4130_sensor_demo_console.png
   :width: 500

===========================
Sensor Channels Calibration
===========================

It is possible to calibrate the device channels which are connected to external
sensors. The sensors calibration (gain and offset) is done by executing the python
script “ad4130_calibration.py”.


.. IIO Firmware Structure

.. include:: /source/tinyiiod/iio_firmware_structure.rst

=======
Support
=======

Feel free to ask questions in the `EngineerZone <https://ez.analog.com/data_converters>`_
