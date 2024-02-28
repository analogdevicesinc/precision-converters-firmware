AD7606 IIO Application
----------------------

==================
Supported Hardware
==================

**Supported Devices:**

* `AD7606B <https://www.analog.com/en/products/ad7606b.html>`_

* `AD7606C-16 <https://www.analog.com/en/products/ad7606c-16.html>`_

* `AD7606C-18 <https://www.analog.com/en/products/ad7606c-18.html>`_

* `AD7605-4 <https://www.analog.com/en/products/ad7605-4.html>`_

* `AD7606-4 <https://www.analog.com/en/products/ad7606-4.html>`_

* `AD7606-6 <https://www.analog.com/en/products/ad7606-6.html>`_

* `AD7606 <https://www.analog.com/en/products/ad7606.html>`_

* `AD7608 <https://www.analog.com/en/products/ad7608.html>`_

* `AD7609 <https://www.analog.com/en/products/ad7609.html>`_

**Supported Evaluation Boards:**

* `EVAL-AD7606B-FMCZ <https://www.analog.com/en/design-center/evaluation-hardware-and-software/evaluation-boards-kits/EVAL-AD7606B-FMCZ.html#eb-overview>`_

* `EVAL-AD7606CFMCZ <https://www.analog.com/en/design-center/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad7606c-18.html>`_

* `EVAL-AD7605-4 <https://www.analog.com/en/design-center/evaluation-hardware-and-software/evaluation-boards-kits/EVAL-AD7605-4.html>`_

* `EVAL-AD7606-4 <https://www.analog.com/en/design-center/evaluation-hardware-and-software/evaluation-boards-kits/EVAL-AD7606-4.html>`_

* `EVAL-AD7606-6 <https://www.analog.com/en/design-center/evaluation-hardware-and-software/evaluation-boards-kits/EVAL-AD7606-6.html>`_

* `EVAL-AD7606 <https://www.analog.com/en/design-center/evaluation-hardware-and-software/evaluation-boards-kits/EVAL-AD7606.html>`_

* `EVAL-AD7608 <https://www.analog.com/en/design-center/evaluation-hardware-and-software/evaluation-boards-kits/EVAL-AD7608.html>`_

* `EVAL-AD7609 <https://www.analog.com/en/design-center/evaluation-hardware-and-software/evaluation-boards-kits/EVAL-AD7609.html>`_

**Supported Carrier Boards:**

* `SDP-K1 With Mbed <https://os.mbed.com/platforms/SDP_K1/>`_

============
Introduction
============

This guide gives an overview of using the IIO firmware application with Analog Devices 
AD7606 Evaluation boards and SDP-K1 (or other compatible) MCU controller board, 
leveraging Mbed-OS as a primary software platform. This firmware application 
leverages the ADI developed IIO (Industrial Input Output) ecosystem to evaluate 
the AD7606 (IIO) device by providing device configuration and data capture support.

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

   .. image:: /source/projects/ad7606_iio/ad7606_hardware_connections.png
      :width: 800

===============
Jumper Settings
===============

**SDP-K1:**

Connect the VIO_ADJUST jumper on the SDP-K1 board to 3.3V position to drive SDP-K1 GPIOs at 3.3V

**EVAL-AD7606B-FMCZ:**

Make below jumper settings on the AD7606B FMC board. 
For other boards, refer respective user manual for more details.

+-----------------------+----------------+---------------------------------------------+
|Jumper                 | Position       | Purpose                                     |
+-----------------------+----------------+---------------------------------------------+
|JP1                    | A              | Ignored in software mode                    |
+-----------------------+----------------+---------------------------------------------+
|JP2                    | B              | The unregulated external supply to the      | 
|                       |                | on-board LDOs is taken from the P4          |
|                       |                | terminal block connector (9V screw terminal)|
+-----------------------+----------------+---------------------------------------------+
|JP3                    | A              | The AD7606B is supplied with 3.3V  VDRIVE   |
|                       |                | from the ADP7118                            |
+-----------------------+----------------+---------------------------------------------+
|JP4                    | A              | Ignored in software mode.                   |
+-----------------------+----------------+---------------------------------------------+
|JP5                    | A              | Serial interface is selected.               |
+-----------------------+----------------+---------------------------------------------+
|JP6                    | B              | Internal reference is enabled and selected. | 
|                       |                | R1 must be unpopulated.                     |
+-----------------------+----------------+---------------------------------------------+
|OS Switches (S1)       | Default (Open) | Open by default. The OS pins are controlled |
|                       |                | and set in the firmware as logic high.      |
+-----------------------+----------------+---------------------------------------------+

.. Communication Interface section:

.. include:: /source/hardware/comm_interface.rst

.. Project Build Section:
    
.. include:: /source/build/project_build.rst

.. IIO Ecosystem Section:
    
.. include:: /source/tinyiiod/iio_ecosystem.rst


Calibrating AD7606B/C Devices
=============================

**ADC Gain Calibration:**

ADC gain calibration can be done in 3 easy steps as mentioned in below diagram. 
The gain calibration needs to be done for selected gain filter register as specified in 
the datasheet (refer 'System Gain Calibration' section from the AD7606B/C datasheet). 
The gain calibration can be done for each channel depending upon the filter 
resistor placed in series with each channel analog input.

*References: Source file: iio_ad7606.c, Function: get_chn_calibrate_adc_gain()*

   .. image:: /source/projects/ad7606_iio/ad7606_gain_calibration.png
      :width: 1000

**ADC Offset Calibration:**

ADC offset calibration should only be done when ADC input is 0V. The purpose is 
to reduce any offset error from the input when analog input is at 0V level. The 
ADC offset calibration can be done for each input channel.
To perform ADC offset calibration, select the 'calibrate_adc_offset' attribute. 
It should automatically perform the calibration. Also, if 'Read' button is pressed, 
the calibration should happen one more time.

*References: Source file: iio_ad7606.c, Function: get_chn_calibrate_adc_offset()*

   .. image:: /source/projects/ad7606_iio/ad7606_offset_calibration.png
      :width: 600


Open Circuit Detection on AD7606B Device
========================================

AD7606B device provides an open circuit detection feature for detecting the open
circuit on each analog input channel of ADC. There are 2 modes to detect open 
circuit on analog inputs (Refer AD7606B datasheet for more details):

* Manual Mode

* Auto Mode

**Manual Mode Open Circuit Detect:**

The manual open circuit detection needs 'Rpd' to be placed at analog input as 
shown in figure below. The firmware is written to perform the open circuit detection 
@50Kohm of Rpd value. The common mode change threshold has been defined as 15 
ADC count in the firmware at above specified configurations (as specified in the datasheet).

*References: Source file: iio_ad7606.c, Function: get_chn_open_circuit_detect_manual()*

   .. image:: /source/projects/ad7606_iio/ad7606_manual_open_circuit.png
      :width: 1000

**Auto Mode Open Circuit Detect:**

The auto open circuit detection on each individual ADC channel can be done by 
performing 3 easy steps mentioned in below screenshot.

*References: Source file: iio_ad7606.c, Function: get_chn_open_circuit_detect_auto()*

   .. image:: /source/projects/ad7606_iio/ad7606_auto_open_circuit.png
      :width: 1000


Diagnostic Multiplexer on AD7606B/C Devices
===========================================

Using diagnostic multiplexer on AD7606B/C devices, the internal analog inputs 
can be sampled to provide a diagnostic voltages and parameters on IIO client 
application such as reference voltage (vref), DLO voltage (ALDO/DLDO), 
temperature and drive voltage (vdrive).

*Note: The diagnostic mux control must operate when input range is +/-10V*

   .. image:: /source/projects/ad7606_iio/ad7606_diagnostic_mux.png
      :width: 1000


.. IIO Firmware Structure

.. include:: /source/tinyiiod/iio_firmware_structure.rst

=======
Support
=======

Feel free to ask questions in the `EngineerZone <https://ez.analog.com/data_converters>`_
