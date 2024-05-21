=======================
Using the IIO Ecosystem
=======================

IIO (Industrial Input/Output) is a flexible ecosystem that allows various
client tools to communicate with IIO device to configure the device, capture device
data, generate waveforms, access registers, etc. Below diagram demonstrates the
high level architecture of IIO ecosystem.

`<https://wiki.analog.com/resources/tools-software/linux-software/libiio>`_

   .. image:: /source/tinyiiod/flexible_iio_ecosystem.png
      :width: 600

==========
IIO Tools:
==========

:doc:`IIO Oscilloscope </source/iio_osc/iio_osc>`
ADI IIO Oscilloscope is a cross platform GUI application, which demonstrates how
to interface different evaluation boards within an IIO ecosystem. It supports
raw data capture, FFT analysis, DMM measurement, device configuration, register 
read/write and data streaming.

:doc:`pyadi-iio: Python interfaces </source/python/python>`
Analog Devices python interfaces for hardware with Industrial I/O drivers. It
provides python based scripts for raw data capture, device configuration 
and register read/write.

`ACE <https://www.analog.com/en/design-center/evaluation-hardware-and-software/evaluation-development-platforms/ace-software.html>`_
ADI's "Analysis, Control, Evaluation" (ACE) is a desktop software application 
which allows the evaluation and control of multiple evaluation systems.

`Precision Converters MATLAB Toolbox <https://github.com/analogdevicesinc/PrecisionToolbox>`_
Toolbox created by ADI to be used with MATLAB and Simulink with ADI Precision products.

`IIO Command Line <https://wiki.analog.com/resources/tools-software/linux-software/libiio/iio_info>`_
Command line interface for accessing IIO device parameters.

:doc:`Pocketlab </source/pocketlab/pocketlab>`
ADI Pocketlab is a display based GUI client. It supports
raw data capture, FFT analysis, DMM measurement, device configuration, register 
read/write.

.. note::

    These are the general evaluation and prototyping tools for Precision Converters
    but not all converters are supported. In some cases these tools provide generic 
    IIO support (e.g. ACE, IIO Oscilloscope) and can provide basic functionality 
    with any IIO based system. In other cases if a part is not currently supported,
    it is possible to add support for converters that you need due to the open 
    source nature of the tools.
