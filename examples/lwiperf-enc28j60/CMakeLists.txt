set(TARGET_NAME enc28j60_lwiperf)
set(LWIP_PATH ${PICO_SDK_PATH}/lib/lwip)
set(TARGET_FILES    
    ${TARGET_NAME}.c
    enc28j60.c
    #${LWIP_PATH}/src/apps/lwiperf/lwiperf.c
    )

add_executable(${TARGET_NAME} ${TARGET_FILES})

# This is required to include lwipopts.h
target_include_directories(pico_lwip INTERFACE        
        ${PORT_DIR}/enc28j60
        )

target_link_libraries(${TARGET_NAME} PRIVATE
        pico_stdlib
        hardware_spi
        hardware_dma
        ENC28J60_FILES
        pico_lwip_iperf
        )

pico_enable_stdio_usb(${TARGET_NAME} 0)
pico_enable_stdio_uart(${TARGET_NAME} 1)

pico_add_extra_outputs(${TARGET_NAME})
