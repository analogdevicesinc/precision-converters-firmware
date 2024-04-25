AD717x / AD411x Console Application
"""""""""""""""""""""""""""""""""""

==================
Supported Hardware
==================

**Supported Devices:**

* `AD4111 <https://www.analog.com/en/products/ad4111.html>`_ `AD4112 <https://www.analog.com/en/products/ad4112.html>`_ `AD4114 <https://www.analog.com/en/products/ad4114.html>`_ `AD4115 <https://www.analog.com/en/products/ad4115.html>`_ `AD4116 <https://www.analog.com/en/products/ad4116.html>`_
* `AD7172-2 <https://www.analog.com/en/products/ad7172.html>`_  `AD7172-4 <https://www.analog.com/en/products/ad7172-4.html>`_
* `AD7173-8 <https://www.analog.com/en/products/ad7173-8.html>`_ `AD7175-2 <https://www.analog.com/en/products/ad7175-2.html>`_ `AD7175-8 <https://www.analog.com/en/products/ad7175-8.html>`_
* `AD7176-2 <https://www.analog.com/en/products/ad7176-2.html>`_
* `AD7177-2 <https://www.analog.com/en/products/ad7177-2.html>`_

**Supported Evaluation Boards:**

* `EVAL-AD4111 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad4111.html>`_ `EVAL-AD4112 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad4112.html>`_ `EVAL-AD4114 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad4114.html>`_ `EVAL-AD4115 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad4115.html>`_ `EVAL-AD4116 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad4116.html>`_
* `EVAL-AD7172-2 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad7172-2.html>`_  `EVAL-AD7172-4 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad7172-4.html>`_
* `EVAL-AD7173-8 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad7173-8sdz.html>`_ `EVAL-AD7175-2 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad7175-2.html>`_ `EVAL-AD7175-8 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad7175-8.html>`_
* `EVAL-AD7176-2 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad7176-2.html>`_
* `EVAL-AD7177-2 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad7177-2.html>`_

**Supported Carrier Boards:**

* `SDP-K1 With Mbed <https://os.mbed.com/platforms/SDP_K1/>`_

============
Introduction
============

The AD717x/AD411x family offer a complete integrated Sigma-Delta ADC solution which can be used in high precision, low noise single channel applications (Life Science measurements) or higher speed multiplexed applications (Factory Automation PLC Input modules).
This page gives an overview of using the AD717x/AD411x firmware example with SDP-K1 EVAL board, interfacing with various EVAL boards supporting AD711x/AD411x family devices. 
The firmware example comprises 3 layers of software (from top to bottom): Console Application Layer, Device No-OS Layer and Platform Drivers (Mbed-OS) layer.

   .. image:: /source/projects/ad717x_console/ad717x_architecture.jpg
      :width: 200

The application layer uses the ADI Console Libraries to create console based User Interactive (UI). 
The middle layer of No-OS device library have device specific APIs to interface with AD717x/AD411x device.
These APIs allows direct access to device register map in order to read/write device registers. The bottom layer of Platform
Drivers is responsible for Low Level Interface. The platform drivers uses mbed-os libraries to access low level peripheral 
(like GPIOs, SPI, I2C, etc).

The Mbed Platform simplifies the overall software development process by providing the low level driver support. This reduces 
the hardware dependency as any Mbed enabled board can be used with same firmware with little modifications (precisely changing a pin mapping).

=================
Interface Diagram
=================

   .. image:: /source/projects/ad717x_console/ad717x_interface_diagram.jpg
      :width: 500

The EVAL-AD411x/EVAL-AD717x board is connected to SDP-K1 through on-board default 120-pin SDP Connector. The AD717x/AD411x EVAL boards can be powered-up through a SDP-K1 USB supply or from external DC supply, 
depending on the power supply jumper settings. The settings can vary board to board and user must refer user manual of respective EVAL board for ensuring the proper connections. 
The SDP-K1 is connected to PC through an USB cable. The firmware (binary executable) can be loaded into SDP-K1 board through this USB interface from the PC. 
The SDP-K1 acts as a Serial Device (UART) and firmware loaded into it interacts with any serial terminal (like Teraterm, Putty, Coolterm, etc) by configuring terminal for proper serial settings (COM Port, Baud Rate, data bits, etc).

.. note::
   The firmware provides a basic user-interface for interacting with the evaluation-board. All the main functionality of the AD411x/AD711x is provided in the application-code in abstracted form and the user is free to customize the software to suit their own needs for working with the AD711x/AD411x.

.. Useful links Section
.. include:: /source/useful_links.rst

====================
Hardware Connections
====================

   .. image:: /source/projects/ad717x_console/ad717x_hw_connection.jpg
      :width: 400

===============================
Power Supply and USB Connection
===============================

* SDP-K1- Connect the VIO_ADJUST jumper on the SDP-K1 board to 3.3V position to drive SDP-K1 GPIOs at 3.3V
* EVAL-AD4111SDZ Board (AD4111/AD4112 chip)- Connect PWR (LK3) connector to USB_SUPP (B) position to power-up device from the SDP-K1 USB.

.. note::
    The settings can vary board to board and user must refer user manual of respective EVAL board for ensuring the proper connections.

SDP-K1 is powered through USB connection from the PC. SDP-K1 acts as a Serial device when connected to PC, which creates a COM Port to connect to Serial Terminals like Teraterm, Putty, etc. 
The COM port assigned to a device can be seen through the device manager for windows based OS.

   .. image:: /source/projects/ad717x_console/com_port_sdp-k1.jpg
      :width: 300

====================
AD717x Mbed Firmware
====================
This section briefs on the usage of MBED firmware. This also explains the steps to compile and build the application using
mbed and make based build.

The software execution sequence for the AD717x/AD411x Firmware Example is shown below. This is a blocking application as it waits for user input over serial interface (UART). 
The input is scanned and processed through 'adi console libraries'. The menu functionality is executed from ad717x_conole_app.c file. The application layer talks with No-OS layer 
for device registers and data access. The No-OS layer interfaces with Platform Drivers layer for accessing low level peripherals. As name suggests, this layer is platform dependent.
AD717x/AD411x firmware uses Mbed libraries within Platform Drivers layer.

   .. image:: /source/projects/ad717x_console/sequence_diagram_ad717x.jpg
      :width: 700

.. Project Build Section:
    
.. include:: /source/build/project_build.rst

===========
Quick Start
===========

If you have some familiarity with the Mbed platform, the following is a basic list of steps required to start running the code, 
see below for more detail:

* Connect the AD717x/AD411x EVAL-board to the SDP-K1 controller board.
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
The AD711x/AD411x firmware example is configured to have following serial settings:

Baud rate: 230400
Data bits: 8-bits
Parity: None
Stop bits: 1

Configure your serial terminal (`Tera Term <https://osdn.net/projects/ttssh2/releases/>`_) 
for below settings:

   .. image:: /source/projects/ad717x_console/baud_rate_update.png
      :width: 400

The AD717x/AD411x console main menu looks like below (with Tera Term):

   .. image:: /source/projects/ad717x_console/ad4111_console_menu.png
      :width: 400

The firmware is designed to be intuitive to use, and requires little explanation, simply enter a number corresponding to the required command and follow the on-screen prompts.

The firmware comes with an app_config.h file which (at the moment) serves two purposes.
* Select the active device to test.
* Configure the pins you want to use to connect the controller board to the evaluation board.

The firmware supports most products in AD717x/AD411x family, change the #define DEV_ADxxxx found in app_config.h to suit your selected device. 
e.g. #define DEV_AD7111_2 executes the AD7112-2 device functionality.

=======
Support
=======

Feel free to ask questions in the `EngineerZone <https://ez.analog.com/data_converters>`_
