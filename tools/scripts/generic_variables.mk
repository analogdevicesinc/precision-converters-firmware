# Default platform is set as Mbed
PLATFORM = mbed

# IIO client (REMOTE/LOCAL)
IIO_CLIENT = REMOTE

ROOT_DRIVE = $(realpath ../..)
LIBRARIES_PATH = $(ROOT_DRIVE)/libraries
NO-OS = $(LIBRARIES_PATH)/no-OS
PROJECT_APP_PATH = $(realpath .)/app
MBED_OS_DIRECTORY_PATH = $(NO-OS)/libraries/mbed
