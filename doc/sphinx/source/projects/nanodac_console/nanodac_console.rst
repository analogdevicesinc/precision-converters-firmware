nanoDAC+ Console Application
----------------------------

==================
Supported Hardware
==================

**Supported Devices:**

`AD5671r <https://www.analog.com/en/products/ad5671r.html>`_ `AD5672r <https://www.analog.com/en/products/ad5672r.html>`_ `AD5673r <https://www.analog.com/en/products/ad5673r.html>`_ 
`AD5674 <https://www.analog.com/en/products/ad5674.html>`_  `AD5674r <https://www.analog.com/en/products/ad5674r.html>`_  `AD5675r <https://www.analog.com/en/products/ad5675r.html>`_  
`AD5675 <https://www.analog.com/en/products/ad5675.html>`_ 
`AD5676 <https://www.analog.com/en/products/ad5676.html>`_ `AD5676r <https://www.analog.com/en/products/ad5676r.html>`_ `AD5677r <https://www.analog.com/en/products/ad5677r.html>`_ 
`AD5679 <https://www.analog.com/en/products/ad5679.html>`_  `AD5679r <https://www.analog.com/en/products/ad5679r.html>`_  `AD5686 <https://www.analog.com/en/products/ad5686.html>`_ 
`AD5684r <https://www.analog.com/en/products/ad5684r.html>`_ `AD5685r <https://www.analog.com/en/products/ad5685r.html>`_ `AD5686r <https://www.analog.com/en/products/ad5686r.html>`_ 
`AD5687 <https://www.analog.com/en/products/ad5687.html>`_  `AD5687r <https://www.analog.com/en/products/ad5687r.html>`_  `AD5689r <https://www.analog.com/en/products/ad5689r.html>`_ 
`AD5697r <https://www.analog.com/en/products/ad5697r.html>`_ `AD5694 <https://www.analog.com/en/products/ad5694.html>`_ `AD5694r <https://www.analog.com/en/products/ad5694r.html>`_ `AD5695r <https://www.analog.com/en/products/ad5695r.html>`_ 
`AD5696 <https://www.analog.com/en/products/ad5696.html>`_  `AD5696r <https://www.analog.com/en/products/ad5696r.html>`_  `AD5681r <https://www.analog.com/en/products/ad5681r.html>`_ 
`AD5682r <https://www.analog.com/en/products/ad5682r.html>`_ `AD5683 <https://www.analog.com/en/products/ad5683.html>`_ `AD5683r <https://www.analog.com/en/products/ad5683r.html>`_ `AD5691r <https://www.analog.com/en/products/ad5691r.html>`_ 
`AD5692r <https://www.analog.com/en/products/ad5692r.html>`_  `AD5693r <https://www.analog.com/en/products/ad5693r.html>`_  `AD5693 <https://www.analog.com/en/products/ad5693.html>`_ 

**Supported Evaluation Boards:**

`EVAL-AD5676 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad5676.html#eb-overview>`_ `EVAL-AD5677r <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad5677r.html#eb-overview>`_ `EVAL-AD5679r <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad5679r.html#eb-overview>`_
`EVAL-AD5686r <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad5686r.html#eb-overview>`_ `EVAL-AD5684r <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad5684r.html#eb-overview>`_ `EVAL-AD5686r <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad5686r.html#eb-overview>`_  
`EVAL-AD5694r <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad5694r.html#eb-overview>`_ `EVAL-AD5696r <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad5696r.html#eb-overview>`_   `EVAL-AD5683r <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad5683r.html#eb-overview>`_ 
`EVAL-AD5693r <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad5693r.html#eb-overview>`_ 

**Supported Carrier Boards:**

* `SDP-K1 With Mbed <https://os.mbed.com/platforms/SDP_K1/>`_

============
Introduction
============

Analog Devices nanoDAC+® products are low power, single-channel, 16-/14-/12-bit buffered voltage output DACs. This page gives an overview of using the nanodac+® 
firmware example with SDP-K1 EVAL board, interfacing with various EVAL boards supporting nanodac+ family devices.
The firmware example comprises 3 layers of software (from top to bottom): Console Application Layer, Device No-OS Layer and Platform Drivers (Mbed-OS) layer.

   .. image:: /source/projects/nanodac_console/nanodac_software_layers.jpg
      :width: 200

The application layer uses the ADI Console Libraries to create console based User Interactive (UI). The middle layer of No-OS device library have device specific APIs
to interface with nanodac+ device. These APIs allows direct access to device register map in order to read/write device registers. The bottom layer of Platform Drivers is
responsible for Low Level Interface. The platform drivers uses mbed-os libraries to access low level peripheral (like GPIOs, SPI, I2C, etc).
The devices from nanodac+ family uses either SPI or I2C communication interface.

The nanoDac+ Mbed firmware example can be used as a starting point for developing your own code for Analog Devices nanoDAC+ products in your own environment utilizing the 
benefits of the Mbed platform. The Mbed Platform simplifies the overall software development process by providing the low level driver support.
This reduces the hardware dependency as any Mbed enabled board can be used with same firmware with little modifications (precisely changing a pin mapping).

=================
Interface Diagram
=================

   .. image:: /source/projects/nanodac_console/nanodac_interface_diagram.jpg
      :width: 500

The AD568xRSDZ(nanodac+)-EVAL board is connected to SDP-K1 through on-board default 120-pin SDP Connector. The nanodac+ EVAL board can be powered-up through a SDP-K1
USB supply or from external DC supply, depending on the power supply jumper settings. The settings can vary board to board and user must refer user manual of respective 
EVAL board for ensuring the proper connections. Apart from power supply selection option, the EVAL board does provide an options to select Vref level, Gain level, I2C slave address bits-2:1 (A0,A1 pins) 
and other options based on the EVAL board hardware configurations. The SDP-K1 is connected to PC through an USB cable. The firmware (binary executable) can be loaded into SDP-K1 board through this USB interface from the PC. 
The SDP-K1 acts as a Serial Device (UART) and firmware loaded into it interacts with any serial terminal (like Teraterm, Putty, Coolterm, etc) by configuring terminal for proper serial settings (COM Port, Baud Rate, data bits, etc).

.. note:: 
   The firmware provides a basic user-interface for interacting with the evaluation-board. All the main functionality of the nanoDAC+ is provided in the application-code in abstracted form and the user is free to customize the software to suit their own needs for working with the nanoDAC+

.. Useful links Section

.. include:: /source/useful_links.rst

====================
Hardware Connections
====================

   .. image:: /source/projects/nanodac_console/nanodac_hardware_connection.jpg
      :width: 400

**SDP-K1:**

Connect the VIO_ADJUST jumper on the SDP-K1 board to 3.3V position to drive SDP-K1 GPIOs at 3.3V

**EVAL-AD56x86RSDZ Board (AD5696R chip):**

* Connect REF connector to 2.5V position for Vref=2.5V.
* Connect PWRSEL connector to USB_SUPP position to power-up device from the SDP-K1 USB.
* Disconnect/Open the P1 jumper to open the connection of VDD and VLOGIC. Use this option when using the SDP board.

.. note:: 
    The settings can vary board to board and user must refer user manual of respective EVAL board for ensuring the proper connections.

SDP-K1 is powered through USB connection from the PC. SDP-K1 acts as a Serial device when connected to PC, which creates a COM Port to connect to Serial Terminals like Teraterm, Putty, etc.
The COM port assigned to a device can be seen through the device manager for windows based OS.

   .. image:: /source/projects/nanodac_console/com_port_sdp-k1.jpg
      :width: 300

======================
nanoDAC+ Mbed Firmware
======================

This section briefs on the usage of MBED firmware. This also explains the steps to compile and build the application using
mbed and make based build.

The software execution sequence for the nanodac+ Firmware Example is shown below. This is a blocking application as it waits for user input over serial interface (UART). 
The input is scanned and processed through 'adi console libraries'. The menu functionality is executed from nanodac_conole_app.c file. The application layer talks with No-OS layer 
for device registers and data access. The No-OS layer interfaces with Platform Drivers layer for accessing low level peripherals. 
As name suggests, this layer is platform dependent. nanodac+ firmware uses Mbed libraries within Platform Drivers layer.

   .. image:: /source/projects/nanodac_console/nanodac_software_sequence.jpg
      :width: 700

.. Project Build Section:
    
.. include:: /source/build/project_build.rst

===========
Quick Start
===========

If you have some familiarity with the Mbed platform, the following is a basic list of steps required to start running the code, 
see below for more detail:

* Connect the nanodac+ EVAL-board to the SDP-K1 controller board.
* Connect the SDP-K1 controller board to your computer over USB.
* Follow the steps mentioned in the Build Guide section above.
* Start up a serial terminal emulator (e.g. Tera Term)
   * Find the com-port your controller board is connected on and select it.
   * Set the baud-rate for 230400
   * Reset the controller board and connect.
* Use the menu provided over the terminal window to access the evaluation board.

==================
Using the Firmware
==================

The nanodac+ firmware example is configured to have following serial settings:

Baud rate: 230400
Data bits: 8-bits
Parity: None
Stop bits: 1

Configure your serial terminal (`Tera Term <https://osdn.net/projects/ttssh2/releases/>`_) 
for below settings:

   .. image:: /source/projects/nanodac_console/baud_rate_update.png
      :width: 400

The nanodac+ console main menu looks like below (with Tera Term):

   .. image:: /source/projects/nanodac_console/nanodac_console_menu.jpg
      :width: 400

The firmware is designed to be intuitive to use, and requires little explanation, simply enter a number corresponding to the required command and follow the on-screen prompts.

The firmware comes with an app_config.h file which (at the moment) serves two purposes.
* Select the active device to test.
* Configure the pins you want to use to connect the controller board to the evaluation board.

The firmware supports most products in nanoDAC+ family, change the #define DEV_ADxxxxx found in app_config.h to suit your selected device. 
The products supported are enumerated in the ad5686_type, which is an enum found in AD5686.h, the firmware defaults to the AD5686R device.

=======
Support
=======

Feel free to ask questions in the `EngineerZone <https://ez.analog.com/data_converters>`_
