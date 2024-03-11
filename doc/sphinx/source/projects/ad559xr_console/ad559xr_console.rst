AD559XR Console Application
---------------------------

==================
Supported Hardware
==================

**Supported Devices:**

* `AD5592R <https://www.analog.com/en/products/ad5592r.html>`_ 
* `AD5593R <https://www.analog.com/en/products/ad5593r.html>`_

**Supported Evaluation Boards:**

* `EVAL-AD5592R <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/EVAL-AD5592R.html>`_ 
* `EVAL-AD5593R <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/Eval-ad5593r.html#eb-overview>`_ 

**Supported Carrier Boards:**

* `SDP-K1 With Mbed <https://os.mbed.com/platforms/SDP_K1/>`_

============
Introduction
============

The AD5592R/ AD5593R offer 8-Channel, 12-Bit, configurable ADC/DAC with on-chip reference and either SPI (AD5592R) or I2C interfaces (AD5593R). This page gives an overview of using the AD5592R/AD5593R firmware example with SDP-K1 EVAL board. The firmware example comprises 3 layers of software (from top to bottom): Console Application Layer, Device No-OS Layer and Platform Drivers (Mbed-OS) layer.

   .. image:: /source/projects/ad559xr_console/ad5592r_software_architecture.jpg
      :width: 200

The application layer uses the ADI Console Libraries to create console-based User Interactive (UI). The middle layer of No-OS device library has device specific APIs to interface with AD5592R/93R devices. These APIs allows direct access to device register map in order to read/write device registers. The bottom layer of Platform Drivers is responsible for Low Level Interface. The platform drivers use mbed-os libraries to access low level peripheral (like GPIOs, SPI, I2C, etc). The devices from AD5592R/AD5593R family use SPI and I2C communication interfaces respectively.

The Mbed Platform simplifies the overall software development process by providing the low-level driver support. This reduces the hardware dependency as any Mbed enabled board can be used with same firmware with little modifications (changing a pin mapping).

=================
Interface Diagram
=================

   .. image:: /source/projects/ad559xr_console/ad5592r_interface.jpg
      :width: 600

The EVAL-AD5592R is connected to SDP-K1 through the on-board default 120-pin SDP connector. Meanwhile, the EVAL-AD5593R is connected using the K1's Arduino Header and jumper cables.

Both evaluation boards have 8 I/O pins available on the SMB connectors - I/O 0-7. These connectors are analog inputs or outputs depending on whether the I/Ox pin is configured as ADCs or DACs, or as digital inputs or outputs if the I/Ox pin is configured as a GPIO. 

The SDP-K1 is connected to PC through USB cable and appears as a USB disk device. The firmware can be loaded into SDP-K1 board through this USB interface from PC, by copying a firmware image file onto the USB disk.

.. Useful links Section

.. include:: /source/useful_links.rst

====================
Hardware Connections
====================

The SDP-K1 is powered by the USB connection to a PC. The SDP-K1 appears as a USB serial device, and the host PC creates a serial or COM Port that can be connected to connect by Terminal software such as Teraterm, Putty, etc. The serial port assigned to a device can be found using the Device Manager for a Windows based OS.

   .. image:: /source/projects/ad559xr_console/ad5592r_connections.jpg
      :width: 600

The EVAL-AD5592R-1SDZ can be connected to the SDP-K1 using the 120-way header and is used for the SPI signals. A separate VDD power supply connection is required, either 5V or 3.3V. This can be wired from the corresponding pin on the SDP-K1 Arduino power header to the J2-1 'VDD' screw terminal.

* If using the on-board ADR431 external reference, a 5V supply is required to provide enough headroom for the reference.
* If using the AD5592R internal reference, a 3.3V supply is sufficient.

For the firmware example code set the links as follows

* LK1-B, selects external VDD supply on J2-1
* LK3-A, selects ADR431 as reference
* LK11-A, selects 3.3V from SDP as Vlogic supply
* LK2, LK4 to LK9, LK20 - when fitted, connects pull down resistor to ground. Remove link depending on how the I/O pins are configured and used.

   .. image:: /source/projects/ad559xr_console/ad5593r_connections.jpg
      :width: 400

The EVAL-AD5593RSDZ can be connected to the SDP-K1 using fly wires from the Arduino header to the signal pins available on J5 and other headers. The VDD power supply connection can be either 5V or 3.3V. This can be wired from the corresponding pin on the SDP-K1 Arduino power header to the J2-1 'VDD' screw terminal.

* If using the on-board ADR431 external reference, a 5V supply is required to provide enough headroom for the reference.
* If using the AD5592R internal reference, a 3.3V supply is sufficient.

The connections to be made between the SDP-K1 and the EVAL-AD55923RSDZ are as follows:

+-----------------------+-----------------------------------+
|SDP-K1 Arduino Header  | EVAL-AD5593RSDZ                   | 
+-----------------------+-----------------------------------+
|SCL/D15                | J5-2 'SCLK'                       |
+-----------------------+-----------------------------------+
|SDA/D14                | J5-3 'SDI'                        | 
+-----------------------+-----------------------------------+
|5V or 3.3V             | J2-1 'VDD'                        |
+-----------------------+-----------------------------------+
|3.3V                   | J1-1 'EXT_LOGIC'                  | 
+-----------------------+-----------------------------------+
|GND                    | J5-10 'GND'                       | 
+-----------------------+-----------------------------------+

For the firmware example code set the links as follows

* LK1-B, selects external VDD supply on J2-1
* LK3-A, selects ADR431 as reference
* LK11-B, selects external Vlogic supply on J1-1
* LK2, LK4 to LK9, LK20 - when fitted, connects pull down resistor to ground. Remove link depending on how the I/O pins are configured and used.

.. Project Build Section:
    
.. include:: /source/build/project_build.rst

===========
Quick Start
===========

If you have some familiarity with the Mbed platform, the following is a basic list of steps required to start running the code, see below for more detail:

* Connect the AD5592R/AD55593R EVAL-board to the SDP-K1 controller board.
* Connect the SDP-K1 controller board to your computer over USB.
* Go to the link provided above in the 'Build Guide' section and import code into Keil Studio Web IDE.
* Ensure SDP-K1 controller board is selected (top right of online-compiler page).
* Compile the code.
* After a successful compile a binary will be downloaded to your computer - store this on your drive.
* Drag and drop this binary to the USB drive hosted by your controller board.
* Start up a serial terminal emulator (e.g., Tera Term)
* Find the com-port your controller board is connected on and select it.
* Set the baud-rate for 230400
* Reset the controller board and connect.
* Use the menu provided over the terminal window to access the evaluation board.

.. note::
   Selecting between the AD5592R and AD5593R is done in app_config.h file by commenting or uncommenting ID_AD5592R/ID_AD5593R as the ACTIVE_DEVICE

   If the AD5592R/93R is not provided with an external voltage reference, or the on-board voltage reference is not powered, enable the internal reference under [s] General Settings

==================
Using the Firmware
==================

The AD5592R/93R firmware example is configured to have following serial settings:

* Baud rate: 230400
* Data bits: 8-bits
* Parity: None
* Stop bits: 1

Configure your serial terminal (`Tera Term <https://osdn.net/projects/ttssh2/releases/>`_) 
for below settings:

   .. image:: /source/projects/ad559xr_console/braud_rate_update.jpg
      :width: 600

The AD5592R Main Menu looks like below (with Tera Term):

   .. image:: /source/projects/ad559xr_console/ad5592r_teraterm.jpg
      :width: 300

The firmware is designed to be intuitive to use, and requires little explanation, simply enter the letter corresponding to the required command and follow the on-screen prompts.

The console menu application provides the following main features:

* Configure each of the 8 I/O channels as ADCs, DACs, ADCs and DACs, GPI or GPO.
* Enable/Disable general settings such as the Pre-charge buffer or change the ADC/DAC Gain.
* Write voltage values to channels configured as DACs.
* Read values in a conversion sequence from channels set as ADCs and included in a conversion sequence.
* Read status of channels set as GPIO Inputs and toggle status of channels set as GPIO Outputs.
* Perform software reset.
* Read die temperature.

It is hoped that the most common functions of the AD5592R and AD5593R devices are coded, but it's likely that some special functionality is not implemented.

=======
Support
=======

Feel free to ask questions in the `EngineerZone <https://ez.analog.com/data_converters>`_
