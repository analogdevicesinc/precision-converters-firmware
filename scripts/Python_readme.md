# Python Installation and Usage Guide on Windows
## 1. Installing Python (Can be skipped if Python is already installed)

### Option 1: Download from Python.org (Recommended)
1. Download Python 3.11 from [Python 3.11 Downloads](https://www.python.org/downloads/release/python-3110/).
2. Run the installer and **check the box "Add Python to PATH"** during installation.
3. Choose "Install Now" for the default installation.

### Option 2: Using Windows Package Manager (winget)
Open PowerShell as Administrator and run:
```powershell
winget install Python.Python.3.11
```

### Option 3: Using Chocolatey
If you have Chocolatey installed:
```powershell
choco install python --version=3.11.0
```

## 2. Verifying the Installation

Open PowerShell or Command Prompt and run:
```powershell
python --version
```
You should see output like: `Python 3.11.x`


## 3. Installing Dependencies from `requirements.txt`

### Without Virtual Environment

1. Open PowerShell or Command Prompt and navigate to your project directory:
    ```powershell
    cd C:\Projects\PCFW_GH_Doc\precision-converters-firmware\scripts
    ```
2. Install requirements:
    ```powershell
    python -m pip install -r requirements.txt
    ```

### With Virtual Environment (Recommended)

1. Create a virtual environment:
    ```powershell
    python -m venv venv
    ```
2. Activate the environment:
    ```powershell
    venv\Scripts\activate
    ```
    
3. Install requirements:
    ```powershell
    python -m pip install -r requirements.txt
    ```

## 4. Running Example Scripts

Navigate to the `scripts` directory and run a script:
```powershell
python example_script.py
```
Replace `example_script.py` with the actual script name (e.g., `ad353xr_example.py`, `ad4052_example.py`).

## 5. Opening the Python REPL

In PowerShell or Command Prompt, simply type:
```powershell
python
```
You will enter the interactive Python shell.

---

## 6. Deactivate environment when done:
     ```
     deactivate
     ```

**Troubleshooting:**  
- Ensure Python 3.11 is installed and added to your system PATH
- If `python` command is not found, try using `py -3.11` instead
- For virtual environment activation issues on PowerShell, ensure execution policy allows script execution
- Verify all dependencies in `requirements.txt` are compatible with Python 3.11
