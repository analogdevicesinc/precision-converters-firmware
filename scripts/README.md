# Workshop Support Notes: AD353xr & AD405x

## Firmware Build
1. Clone public repo ([repo](https://github.com/analogdevicesinc/precision-converters-firmware)) and build as per documentation: https://analogdevicesinc.github.io/precision-converters-firmware/
2. Flash or run firmware (drag and drop method).

## Requirements:
1. STM32CubeIDE: 6.15.1
2. STM32CubeMX: 6.11.1

## AD405x Setup & Demo


### Hardware Checklist
- AD4050 board
- 0 to 2.5V input

### Modes
- 12 bit to 16 bit with OSR (AD4050)
- 16 bit to 20 bit with OSR (AD4052)
- Sample mode: 1 MSPS
- Burst avg mode: 62.5 KSPS
- OSR: 2
- OSF: 2 MSPS
- Burst filter length: number of samples averaged before giving out the result
- S-rate: internal s-rate at which ADC is sampling the result
- S-rate: ODR max possible (can configure less than that)

### Firmware Demo Steps
Open ACE software: (release/internal versions)
- Initial configuration
- System configuration
- Integrated doc
- Memory map
- Analysis: Time and Freq domain
- Data capture using ACE
- Portability aspects of the firmware
- Same FW for different generics of the family
- Same FW for different clients
- Porting support

## AD353xr Setup & Demo
- 2.5V reference
- 16 bit DAC

### Firmware Demo Steps
1. Use Scopy for signal scoping (connect 1+-, GND).
2. Open ACE software: (release/internal versions)
    - Board view, documentation, initial config, memory map, waveform generation.
    - Create and preview custom waveforms (default, triangular, square).
    - Save data: `C:\Users\SGudla\AppData\Local\Analog Devices\ACE (internal)\ExportData`
    - Data streaming: peak-to-peak, mean, output frequency, analysis.
    - Configure and Improve sampling frequency using streaming mode.
    - Future: custom data upload as .txt (quantized/non-quantized).

```
iio_info -u serial:COM59,230400
iio_attr -u serial:COM59,230400 -c ad4050 voltage0 raw
iio_attr -u serial:COM4,230400 -c ad3531r voltage0 raw
iio_attr -u serial:COM4,230400 -c ad3531r voltage0 raw 25000
```

# Python Support Notes: AD353xr & AD405x

## PyADI installation
```
- pip install pylibiio==0.25
- pip install pyadi-iio
```

## Other requirements (optional)

Numpy
Scipy
Genalyzer

- requirements.txt

```
https://github.com/analogdevicesinc/genalyzer/releases
pip install --index-url https://test.pypi.org/simple/ pylibgenalyzer
```

## CLI scripts.. where to find and when to expect?

https://github.com/adi-innersource/pcs-precision-converters-firmware/tree/develop/scripts

Planned to release by early 2026 to PyADI for public release.

https://confluence.analog.com/display/LPTSWDEV/Using+PyADI+Command+Line+Interface+Tools

## AD353xr Python Support
- Supports normal operation, power down, internal/external reference selection.
- Channel-wise configuration for voltage output.
- Streaming mode with configurable sampling frequency and buffer size.

## AD405x Python Support
- Supports different device variants (AD4050, AD4052).
- Configuration of sampling frequency, enabled channels, and buffer size.
- Reading and writing channel data.

### Workshop Agenda

1. **PyADI Overview**
    - Introduction to PyADI and its capabilities.
    - How to configure attributes and capture data without deep API knowledge.
    - Installation & requirements: `numpy`, `scipy`, `genalyzer`.

2. **AD3530r Hands-On**
    - Using REPL (Read-Eval-Print Loop) for quick experimentation.
    - Opening VS Code in `/FTC_work` and running sample scripts.
    - Running the `ad353xr_example` CLI script from the `/scripts` folder.

3. **Waveform Generation & Visualization**
    - Introduction to `adistream` CLI tool: usage and future plans.
    - Overview of the `adiplot.py` script.
    - Running `adiplot.py` (CLI): exploring available options.

4. **AD405x Data Capture**
    - Running a simple AD405x example to capture data.
    - Introduction and help for the `adilog.py` CLI tool.
    - Capturing data using `adilog.py`.


### Example Python Commands
```
py adilog.py --help

py adilog.py ad405x serial:COM6,230400 -d ad4052
py adilog.py ad405x serial:COM59,230400 -d ad4050
```
### Example Python Commands
```
py adistream.py --help

py adistream.py ad353xr serial:COM60,230400 0 2.5 -d ad3530r
py adistream.py ad353xr serial:COM60,230400 0 2.5 -d ad3530r -cl 1

py adistream.py ad353xr serial:COM4,230400 0 2.5 -d ad3531r
py adistream.py ad353xr serial:COM4,230400 0 2.5 -d ad3531r -w triangular
```

### CLI example script

```
py ad353xr_example.py --help

py ad353xr_example.py --uri serial:COM60,230400
```

### Python API Usage
```python
import adi
device = adi.ad353xr("serial:COM60,230400", "ad3530r")
device.all_ch_operating_mode = "normal_operation"
device.reference_select = "internal_ref"
device.channel[0].raw = 25000
print(device.channel[0].raw)
```

### Python API Usage
```python
import adi
import numpy as np

device = adi.ad405x("serial:COM59,230400", "ad4050")
# User configuration

device.rx_enabled_channels = ["voltage0"]
device.rx_buffer_size = 400
device._rx_data_type = np.uint16

data = device.rx()

# Reading attribute
print(device.sampling_frequency)
# Setting attribute
device.sampling_frequency = 55000
# Reading channel attribute
print(device.channels[0].raw)
```

### 5. AD405x MATLAB Support

- **Introduction**
    - Overview of MATLAB integration for AD405x data capture and analysis.
    - Matlab R2021b
- **Scripting in MATLAB**
    1. Open the `ad405x_dataCapture` example in the `example` folder.
    2. Set the correct COM port for your device.
    3. Add the example folder to the MATLAB path.
    4. Run the script to capture and plot data.

## CI CD:

### CLoudsmith Access:
https://cloudsmith.io/~adi/repos/pcs-precision-converters-firmware/packages/

### Docker Images:
https://github.com/adi-innersource/pcs-precision-converters-firmware/pkgs/container/pcts-precision-converters-firmware

## Libiio v1.0 Overview

### 1. Introduction
- Overview of previous libiio versions and their limitations.

### 2. Architectural Improvements
- Key enhancements in libiio v1.0.
- Benefits and new features.

### 3. Progress & Work Done
- Summary of current development and integration efforts.

### 4. Libiio DLLs & Environment Setup
- New DLLs location:  
    `C:\Program Files (x86)\libiio\lib\libiio`
- Ensure libiio path is added to environment variables.

### 5. Python Testing
- Side-by-side testing using system libiio.
- Steps for running Python tests.

### 6. AD4050 Device Testing

#### Python Workflow

- Set input frequency to 100Hz.
- Use `adilog_ex.py` for plotting.
- Example command:
    ```
    py example.py
    py adilog_ex.py ad405x serial:COM10,230400 -d ad4050
    py adilog_ex.py ad405x serial:COM10,230400 -d ad4050 -s 1000
    ```

#### Virtual Environment Setup
- Example scripts:  
    `C:\Users\SGudla\OneDrive - Analog Devices, Inc\Documents\FTC_work\Python\Python`
1. Open command prompt.
2. Activate virtual environment:
     ```
     venv\Scripts\activate
     ```
3. Update COM port in the script.
4. Install required packages: (if not installed)
     ```
     pip install pyqtgraph PyQt5
     ```
5. Run data capture script:
     - For PyQt GUI:  
         `python adilog_ex.py`
6. Press `Esc` to close the application.
7. Sampling rate configurable (max: 100 kSPS).
8. Input range: 20–500 Hz, 0–2.5 V, 1.25 V offset.
9. Deactivate environment when done:
     ```
     deactivate
     ```


---

Resources

- Libiio: https://github.com/analogdevicesinc/libiio/tree/v0.25
- Installation: https://github.com/analogdevicesinc/libiio/releases
- iio_info: https://wiki.analog.com/resources/tools-software/linux-software/libiio/iio_info

- Packages: Docker: https://github.com/adi-innersource/pcs-precision-converters-firmware/pkgs/container/pcts-precision-converters-firmware

- ACE: https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-development-platforms/ace-software.html

- CLI tools: https://github.com/adi-innersource/pcs-precision-converters-firmware/tree/develop/scripts