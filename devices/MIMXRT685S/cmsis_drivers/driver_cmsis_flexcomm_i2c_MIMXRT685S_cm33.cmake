include_guard()
message("driver_cmsis_flexcomm_i2c component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/fsl_i2c_cmsis.c
)


target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/.
)


include(driver_flexcomm_i2c_dma_MIMXRT685S_cm33)

include(CMSIS_Driver_Include_I2C_MIMXRT685S_cm33)
