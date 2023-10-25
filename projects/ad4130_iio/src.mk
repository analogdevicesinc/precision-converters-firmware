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
CFLAGS += -I$(LIBRARIES_PATH)
CFLAGS += -I$(LIBRARIES_PATH)/stm32_lvgl/lv_port_stm32f769_disco
CFLAGS += -I$(LIBRARIES_PATH)/lvgl
CFLAGS += -I$(LIBRARIES_PATH)/CMSIS-DSP
CFLAGS += -include $(LIBRARIES_PATH)/lv_conf.h
CFLAGS += -include $(LIBRARIES_PATH)/lvgl/lvgl.h
INCS += $(LIBRARIES_PATH)/lv_conf.h
INCS += $(LIBRARIES_PATH)/lvgl/lvgl.h

# Include pocket lab files
SRC_DIRS += $(LIBRARIES_PATH)/precision-converters-library/pocket_lab
SRC_DIRS += $(LIBRARIES_PATH)/precision-converters-library/fft

# Include lvgl source files
SRC_DIRS += $(LIBRARIES_PATH)/lvgl/src

# Include stm32f769ni-disco lvgl portable files
SRC_DIRS += $(LIBRARIES_PATH)/stm32_lvgl/lv_port_stm32f769_disco/hal_stm_lvgl/tft
SRC_DIRS += $(LIBRARIES_PATH)/stm32_lvgl/lv_port_stm32f769_disco/hal_stm_lvgl/touchpad
SRC_DIRS += $(LIBRARIES_PATH)/stm32_lvgl/lv_port_stm32f769_disco/src
SRC_DIRS += $(LIBRARIES_PATH)/stm32_lvgl/lv_port_stm32f769_disco/inc
SRC_DIRS += $(LIBRARIES_PATH)/stm32_lvgl/lv_port_stm32f769_disco/Utilities/Components/adv7533
SRC_DIRS += $(LIBRARIES_PATH)/stm32_lvgl/lv_port_stm32f769_disco/Utilities/Components/ft6x06
SRC_DIRS += $(LIBRARIES_PATH)/stm32_lvgl/lv_port_stm32f769_disco/Utilities/Components/mx25l512
SRC_DIRS += $(LIBRARIES_PATH)/stm32_lvgl/lv_port_stm32f769_disco/Utilities/Components/otm8009a
SRC_DIRS += $(LIBRARIES_PATH)/stm32_lvgl/lv_port_stm32f769_disco/Utilities/Components/wm8994
SRC_DIRS += $(LIBRARIES_PATH)/stm32_lvgl/lv_port_stm32f769_disco/Utilities/Components/Common
SRC_DIRS += $(LIBRARIES_PATH)/stm32_lvgl/lv_port_stm32f769_disco/Utilities/Fonts
SRC_DIRS += $(LIBRARIES_PATH)/stm32_lvgl/lv_port_stm32f769_disco/Utilities/STM32F769I-Discovery

# Include CMSIS-DSP library files
SRC_DIRS += $(LIBRARIES_PATH)/CMSIS-DSP/Source/BasicMathFunctions
SRC_DIRS += $(LIBRARIES_PATH)/CMSIS-DSP/Source/CommonTables
SRC_DIRS += $(LIBRARIES_PATH)/CMSIS-DSP/Source/ComplexMathFunctions
SRC_DIRS += $(LIBRARIES_PATH)/CMSIS-DSP/Source/ControllerFunctions
SRC_DIRS += $(LIBRARIES_PATH)/CMSIS-DSP/Source/FastMathFunctions
SRC_DIRS += $(LIBRARIES_PATH)/CMSIS-DSP/Source/FilteringFunctions
SRC_DIRS += $(LIBRARIES_PATH)/CMSIS-DSP/Source/MatrixFunctions
SRC_DIRS += $(LIBRARIES_PATH)/CMSIS-DSP/Source/StatisticsFunctions
SRC_DIRS += $(LIBRARIES_PATH)/CMSIS-DSP/Source/SupportFunctions
SRC_DIRS += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions
SRC_DIRS += $(LIBRARIES_PATH)/CMSIS-DSP/Include

# Ignoring stm32f769ni-disco lvgl portable files
IGNORED_FILES += $(LIBRARIES_PATH)/stm32_lvgl/lv_port_stm32f769_disco/CMSIS
IGNORED_FILES += $(LIBRARIES_PATH)/stm32_lvgl/lv_port_stm32f769_disco/HAL_Driver
IGNORED_FILES += $(LIBRARIES_PATH)/stm32_lvgl/lv_port_stm32f769_disco/ide
IGNORED_FILES += $(LIBRARIES_PATH)/stm32_lvgl/lv_port_stm32f769_disco/startup
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/stm32_lvgl/lv_port_stm32f769_disco/Utilities/STM32F769I-Discovery/stm32f769i_discovery_audio.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/stm32_lvgl/lv_port_stm32f769_disco/src/main.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/stm32_lvgl/lv_port_stm32f769_disco/src/syscalls.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/stm32_lvgl/lv_port_stm32f769_disco/src/system_stm32f7xx.c

# Ignoring unused files from CMSIS-DSP library
IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/cmsisdsp
IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/ComputeGraph
IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/ComputeLibrary
IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/PythonWrapper
IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Examples
IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Scripts
IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Testing
IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/BayesFunctions
IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/DistanceFunctions
IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/InterpolationFunctions
IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/QuaternionMathFunctions
IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/SVMFunctions
IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/WindowFunctions
IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/PrivateInclude

ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/BasicMathFunctions/arm_abs_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/BasicMathFunctions/arm_abs_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/BasicMathFunctions/arm_add_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/BasicMathFunctions/arm_add_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/BasicMathFunctions/arm_and_u8.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/BasicMathFunctions/arm_and_u16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/BasicMathFunctions/arm_and_u32.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/BasicMathFunctions/arm_clip_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/BasicMathFunctions/arm_clip_f32.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/BasicMathFunctions/arm_clip_q7.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/BasicMathFunctions/arm_clip_q15.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/BasicMathFunctions/arm_clip_q31.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/BasicMathFunctions/arm_dot_prod_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/BasicMathFunctions/arm_dot_prod_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/BasicMathFunctions/arm_mult_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/BasicMathFunctions/arm_mult_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/BasicMathFunctions/arm_negate_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/BasicMathFunctions/arm_negate_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/BasicMathFunctions/arm_offset_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/BasicMathFunctions/arm_offset_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/BasicMathFunctions/arm_scale_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/BasicMathFunctions/arm_scale_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/BasicMathFunctions/arm_sub_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/BasicMathFunctions/arm_sub_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/BasicMathFunctions/arm_not_u8.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/BasicMathFunctions/arm_not_u16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/BasicMathFunctions/arm_not_u32.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/BasicMathFunctions/arm_or_u8.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/BasicMathFunctions/arm_or_u16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/BasicMathFunctions/arm_or_u32.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/BasicMathFunctions/arm_xor_u8.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/BasicMathFunctions/arm_xor_u16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/BasicMathFunctions/arm_xor_u32.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/BasicMathFunctions/BasicMathFunctions.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/BasicMathFunctions/BasicMathFunctionsF16.c

ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/CommonTables/arm_common_tables_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/CommonTables/arm_const_structs_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/CommonTables/arm_mve_tables.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/CommonTables/arm_mve_tables_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/CommonTables/CommonTables.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/CommonTables/CommonTablesF16.c

ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/ComplexMathFunctions/arm_cmplx_conj_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/ComplexMathFunctions/arm_cmplx_dot_prod_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/ComplexMathFunctions/arm_cmplx_mag_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/ComplexMathFunctions/arm_cmplx_mag_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/ComplexMathFunctions/arm_cmplx_mag_squared_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/ComplexMathFunctions/arm_cmplx_mag_squared_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/ComplexMathFunctions/arm_cmplx_mag_fast_q15.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/ComplexMathFunctions/arm_cmplx_mult_cmplx_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/ComplexMathFunctions/arm_cmplx_mult_cmplx_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/ComplexMathFunctions/arm_cmplx_mult_real_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/ComplexMathFunctions/arm_cmplx_mult_real_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/ComplexMathFunctions/ComplexMathFunctions.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/ComplexMathFunctions/ComplexMathFunctionsF16.c

ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/ControllerFunctions/ControllerFunctions.c

ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/FastMathFunctions/arm_atan2_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/FastMathFunctions/arm_atan2_f32.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/FastMathFunctions/arm_atan2_q15.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/FastMathFunctions/arm_atan2_q31.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/FastMathFunctions/arm_divide_q15.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/FastMathFunctions/arm_divide_q31.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/FastMathFunctions/arm_vexp_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/FastMathFunctions/arm_vexp_f32.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/FastMathFunctions/arm_vexp_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/FastMathFunctions/arm_vinverse_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/FastMathFunctions/arm_vlog_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/FastMathFunctions/arm_vlog_f32.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/FastMathFunctions/arm_vlog_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/FastMathFunctions/arm_vlog_q15.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/FastMathFunctions/arm_vlog_q31.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/FastMathFunctions/FastMathFunctions.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/FastMathFunctions/FastMathFunctionsF16.c

ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/SupportFunctions/arm_copy_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/SupportFunctions/arm_copy_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/SupportFunctions/arm_fill_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/SupportFunctions/arm_fill_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/SupportFunctions/arm_float_to_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/SupportFunctions/arm_float_to_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/SupportFunctions/arm_q7_to_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/SupportFunctions/arm_q7_to_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/SupportFunctions/arm_q15_to_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/SupportFunctions/arm_q15_to_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/SupportFunctions/arm_q31_to_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/SupportFunctions/arm_q31_to_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/SupportFunctions/arm_f16_to_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/SupportFunctions/arm_f16_to_q15.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/SupportFunctions/arm_f16_to_float.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/SupportFunctions/arm_f64_to_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/SupportFunctions/arm_f64_to_float.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/SupportFunctions/arm_f64_to_q15.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/SupportFunctions/arm_f64_to_q31.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/SupportFunctions/arm_f64_to_q7.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/SupportFunctions/arm_barycenter_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/SupportFunctions/arm_barycenter_f32.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/SupportFunctions/arm_bitonic_sort_f32.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/SupportFunctions/arm_bubble_sort_f32.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/SupportFunctions/arm_heap_sort_f32.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/SupportFunctions/arm_insertion_sort_f32.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/SupportFunctions/arm_merge_sort_f32.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/SupportFunctions/arm_merge_sort_init_f32.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/SupportFunctions/arm_quick_sort_f32.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/SupportFunctions/arm_selection_sort_f32.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/SupportFunctions/arm_sort_f32.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/SupportFunctions/arm_sort_init_f32.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/SupportFunctions/arm_weighted_sum_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/SupportFunctions/arm_weighted_sum_f32.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/SupportFunctions/SupportFunctions.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/SupportFunctions/SupportFunctionsF16.c

ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/FilteringFunctions/arm_biquad_cascade_df1_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/FilteringFunctions/arm_biquad_cascade_df1_init_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/FilteringFunctions/arm_biquad_cascade_df2T_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/FilteringFunctions/arm_biquad_cascade_df2T_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/FilteringFunctions/arm_biquad_cascade_df2T_init_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/FilteringFunctions/arm_biquad_cascade_df2T_init_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/FilteringFunctions/arm_biquad_cascade_stereo_df2T_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/FilteringFunctions/arm_biquad_cascade_stereo_df2T_init_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/FilteringFunctions/arm_correlate_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/FilteringFunctions/arm_correlate_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/FilteringFunctions/arm_fir_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/FilteringFunctions/arm_fir_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/FilteringFunctions/arm_fir_init_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/FilteringFunctions/arm_fir_init_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/FilteringFunctions/arm_levinson_durbin_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/FilteringFunctions/FilteringFunctions.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/FilteringFunctions/FilteringFunctionsF16.c

ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/MatrixFunctions/arm_householder_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/MatrixFunctions/arm_householder_f32.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/MatrixFunctions/arm_householder_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/MatrixFunctions/arm_mat_add_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/MatrixFunctions/arm_mat_cholesky_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/MatrixFunctions/arm_mat_cholesky_f32.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/MatrixFunctions/arm_mat_cholesky_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/MatrixFunctions/arm_mat_cmplx_mult_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/MatrixFunctions/arm_mat_cmplx_trans_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/MatrixFunctions/arm_mat_init_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/MatrixFunctions/arm_mat_init_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/MatrixFunctions/arm_mat_inverse_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/MatrixFunctions/arm_mat_inverse_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/MatrixFunctions/arm_mat_ldlt_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/MatrixFunctions/arm_mat_mult_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/MatrixFunctions/arm_mat_mult_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/MatrixFunctions/arm_mat_qr_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/MatrixFunctions/arm_mat_qr_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/MatrixFunctions/arm_mat_scale_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/MatrixFunctions/arm_mat_solve_lower_triangular_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/MatrixFunctions/arm_mat_solve_lower_triangular_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/MatrixFunctions/arm_mat_solve_upper_triangular_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/MatrixFunctions/arm_mat_solve_upper_triangular_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/MatrixFunctions/arm_mat_sub_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/MatrixFunctions/arm_mat_sub_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/MatrixFunctions/arm_mat_trans_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/MatrixFunctions/arm_mat_trans_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/MatrixFunctions/arm_mat_vec_mult_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/MatrixFunctions/MatrixFunctions.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/MatrixFunctions/MatrixFunctionsF16.c

ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/StatisticsFunctions/arm_absmax_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/StatisticsFunctions/arm_absmax_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/StatisticsFunctions/arm_absmax_no_idx_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/StatisticsFunctions/arm_absmax_no_idx_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/StatisticsFunctions/arm_absmin_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/StatisticsFunctions/arm_absmin_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/StatisticsFunctions/arm_absmin_no_idx_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/StatisticsFunctions/arm_absmin_no_idx_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/StatisticsFunctions/arm_accumulate_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/StatisticsFunctions/arm_accumulate_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/StatisticsFunctions/arm_entropy_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/StatisticsFunctions/arm_entropy_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/StatisticsFunctions/arm_kullback_leibler_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/StatisticsFunctions/arm_kullback_leibler_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/StatisticsFunctions/arm_logsumexp_dot_prod_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/StatisticsFunctions/arm_logsumexp_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/StatisticsFunctions/arm_max_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/StatisticsFunctions/arm_max_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/StatisticsFunctions/arm_max_no_idx_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/StatisticsFunctions/arm_max_no_idx_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/StatisticsFunctions/arm_mean_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/StatisticsFunctions/arm_mean_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/StatisticsFunctions/arm_min_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/StatisticsFunctions/arm_min_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/StatisticsFunctions/arm_min_no_idx_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/StatisticsFunctions/arm_min_no_idx_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/StatisticsFunctions/arm_mse_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/StatisticsFunctions/arm_mse_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/StatisticsFunctions/arm_power_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/StatisticsFunctions/arm_power_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/StatisticsFunctions/arm_rms_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/StatisticsFunctions/arm_std_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/StatisticsFunctions/arm_std_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/StatisticsFunctions/arm_var_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/StatisticsFunctions/arm_var_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/StatisticsFunctions/StatisticsFunctions.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/StatisticsFunctions/StatisticsFunctionsF16.c

ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_bitreversal_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_cfft_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_cfft_radix4_f32.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_cfft_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_cfft_init_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_cfft_radix4_init_f32.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_cfft_init_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_cfft_init_q15.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_cfft_init_q31.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_cfft_q15.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_cfft_q31.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_cfft_radix2_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_cfft_radix2_f32.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_cfft_radix2_init_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_cfft_radix2_init_f32.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_cfft_radix2_init_q15.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_cfft_radix2_init_q31.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_cfft_radix2_q15.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_cfft_radix2_q31.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_cfft_radix4_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_cfft_radix4_init_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_cfft_radix4_init_q15.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_cfft_radix4_init_q31.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_cfft_radix4_q15.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_cfft_radix4_q31.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_cfft_radix8_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_dct4_f32.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_dct4_init_f32.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_dct4_init_q15.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_dct4_init_q31.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_dct4_q15.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_dct4_q31.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_mfcc_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_mfcc_f32.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_mfcc_init_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_mfcc_init_f32.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_mfcc_init_q15.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_mfcc_init_q31.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_mfcc_q15.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_mfcc_q31.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_rfft_f32.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_rfft_fast_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_rfft_fast_f32.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_rfft_fast_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_rfft_fast_init_f16.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_rfft_fast_init_f32.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_rfft_fast_init_f64.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_rfft_init_f32.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_rfft_init_q15.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_rfft_init_q31.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_rfft_q15.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/arm_rfft_q31.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/TransformFunctions.c
ALL_IGNORED_FILES += $(LIBRARIES_PATH)/CMSIS-DSP/Source/TransformFunctions/TransformFunctionsF16.c
endif
endif

# Extra Macros
override NEW_CFLAGS += -DACTIVE_PLATFORM=MBED_PLATFORM
