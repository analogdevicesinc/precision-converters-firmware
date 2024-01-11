=======================
Communication Interface
=======================

.. note::

   For data transmission to IIO clients, IIO firmware applications uses *Virtual Serial*
   Or *UART* as primary communication links. Firmware by default uses the Virtual Serial
   interface for higher speed data transmission as SDP-K1 MCU board and Mbed firmware supports
   both Virtual Serial and UART interface. If you target a different MCU board that does not 
   support Virtual Serial, just set UART as communication link in the firmware (app_config.h file).

SDP-K1 is powered through USB connection from the computer. SDP-K1 MCU board 
acts as a serial device when connected to PC, which creates a serial ports to connect to IIO 
client application running on PC. The serial port assigned to a device can be seen 
through the device manager for windows-based OS as shown below:

.. image:: /source/hardware/serial_ports_view.png
   :width: 350

.. note::

   The serial port naming is used differently on different operating systems.
   For example, Linux uses terms such as dev/ttyUSB* and Mac uses terms such as dev/tty.USB*.
   Please check serial port naming for your selected OS.

**Identifying Virtual Serial Port (Windows-OS):**

To identify if serial port is virtual, right click on the port name and select 'Properties'.
Select 'Events' from menu option and check for below highlighted VIDs/PIDs and 
firmware name which is currently running on MCU.

.. image:: /source/hardware/identifying_virtual_serial_port.png
   :width: 350

SDP-K1 can support high speed Virtual Serial USB interface, so by default Virtual Serial 
is configured as default interface in the Mbed firmware. The interface can be set to 
Physical (UART) serial port by defining macro **USE_PHY_COM_PORT** in the app_config.h file.

.. code-block:: C

   /* Enable the UART/VirtualCOM port connection (default VCOM) */
   //#define USE_PHY_COM_PORT		// Uncomment to select UART
