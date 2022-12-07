############### Variable Related to Root of Repository ####################
ROOT_DIR_PATH = $(realpath ..)
ROOT_NAME = $(notdir $(ROOT_DIR_PATH))
LIBRARIES_PATH = $(ROOT_DIR_PATH)/libraries

############### Project Related Variables ###################
PROJECT_PATH = $(realpath .)
NAME_OF_PROJECT = $(notdir $(PROJECT_PATH))
PROJECT_APP	= $(PROJECT_PATH)/app
PROJECT_TARGET_FILE	= $(BUILD_DIRECTORY)/.project.target