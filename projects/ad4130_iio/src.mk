# All the .c and .cpp and .h Files in SRC_DIRS are used in Build (Recursive)
SRC_DIRS += $(PROJECT_APP_PATH)
SRC_DIRS += $(LIBRARIES_PATH)/no-OS/drivers/afe/ad413x
SRC_DIRS += $(LIBRARIES_PATH)/no-OS/drivers/eeprom/24xx32a
SRC_DIRS += $(LIBRARIES_PATH)/no-OS/drivers/api
SRC_DIRS += $(LIBRARIES_PATH)/no-OS/drivers/platform/mbed
SRC_DIRS += $(LIBRARIES_PATH)/no-OS/iio
SRC_DIRS += $(LIBRARIES_PATH)/no-OS/util
SRC_DIRS += $(LIBRARIES_PATH)/no-OS/include
SRC_DIRS += $(LIBRARIES_PATH)/precision-converters-library/common
SRC_DIRS += $(LIBRARIES_PATH)/precision-converters-library/board_info
SRC_DIRS += $(LIBRARIES_PATH)/precision-converters-library/sdp_k1_sdram
SRC_DIRS += $(LIBRARIES_PATH)/precision-converters-library/tempsensors

# Source files for building pocket lab application
ifeq 'DISCO_F769NI' '$(TARGET_BOARD)'
ifeq 'LOCAL' '$(IIO_CLIENT)'
# Extra Macros
override NEW_CFLAGS += -DACTIVE_IIO_CLIENT=IIO_CLIENT_LOCAL
override NEW_CFLAGS += -DUSE_PHY_COM_PORT

# Add the include dependency for files with relative paths
CFLAGS += -I$(LIBRARIES_PATH)/
CFLAGS += -I$(LIBRARIES_PATH)/stm32_lvgl/lv_port_stm32f769_disco/
CFLAGS += -include $(LIBRARIES_PATH)/lv_conf.h
INCS += $(LIBRARIES_PATH)/lv_conf.h
INCS += $(LIBRARIES_PATH)/lvgl/lvgl.h

SRC_DIRS += $(LIBRARIES_PATH)/precision-converters-library/pocket_lab

SRC_DIRS += $(LIBRARIES_PATH)/stm32_lvgl/BSP_DISCO_F769NI/Drivers/BSP/STM32F769I-Discovery
SRC_DIRS += $(LIBRARIES_PATH)/stm32_lvgl/BSP_DISCO_F769NI/Drivers/BSP/Components
SRC_DIRS += $(LIBRARIES_PATH)/stm32_lvgl/BSP_DISCO_F769NI/Utilities

SRC_DIRS += $(LIBRARIES_PATH)/lvgl/src
SRC_DIRS += $(LIBRARIES_PATH)/lvgl/demos
SRC_DIRS += $(LIBRARIES_PATH)/lvgl/examples
SRC_DIRS += $(LIBRARIES_PATH)/lvgl/tests
SRC_DIRS += $(LIBRARIES_PATH)/lvgl/env_support
SRC_DIRS += $(LIBRARIES_PATH)/lvgl/docs
SRC_DIRS += $(LIBRARIES_PATH)/lvgl/scripts

SRC_DIRS += $(LIBRARIES_PATH)/stm32_lvgl/lv_port_stm32f769_disco/hal_stm_lvgl
SRC_DIRS += $(LIBRARIES_PATH)/stm32_lvgl/lv_port_stm32f769_disco/src
SRC_DIRS += $(LIBRARIES_PATH)/stm32_lvgl/lv_port_stm32f769_disco/inc

IGNORED_FILES += $(LIBRARIES_PATH)/stm32_lvgl/lv_port_stm32f769_disco/CMSIS
IGNORED_FILES += $(LIBRARIES_PATH)/stm32_lvgl/lv_port_stm32f769_disco/HAL_Driver
IGNORED_FILES += $(LIBRARIES_PATH)/stm32_lvgl/lv_port_stm32f769_disco/ide
IGNORED_FILES += $(LIBRARIES_PATH)/stm32_lvgl/lv_port_stm32f769_disco/startup
IGNORED_FILES += $(LIBRARIES_PATH)/stm32_lvgl/lv_port_stm32f769_disco/Utilities

ALL_IGNORED_FILES += $(LIBRARIES_PATH)/stm32_lvgl/BSP_DISCO_F769NI/Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_audio.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/stm32_lvgl/lv_port_stm32f769_disco/src/main.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/stm32_lvgl/lv_port_stm32f769_disco/src/syscalls.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/stm32_lvgl/lv_port_stm32f769_disco/src/system_stm32f7xx.c
endif
endif

# Extra Macros
override NEW_CFLAGS += -DACTIVE_PLATFORM=MBED_PLATFORM
