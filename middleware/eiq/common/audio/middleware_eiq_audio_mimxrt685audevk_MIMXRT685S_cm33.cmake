include_guard()
message("middleware_eiq_audio_mimxrt685audevk component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/rtxxx/aud_evk/audio_stream.c
    ${CMAKE_CURRENT_LIST_DIR}/rtxxx/audio_capture.cpp
)


target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/rtxxx
)

