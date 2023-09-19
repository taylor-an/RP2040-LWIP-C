set(TARGET_NAME w6x00_iperf)

add_executable(${TARGET_NAME}
        ${TARGET_NAME}.c
        tcp_server.c
        )

target_include_directories(w6x00_iperf PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        )

target_link_libraries(${TARGET_NAME} PRIVATE
        pico_stdlib
        hardware_spi
        hardware_dma
        ETHERNET6_FILES
        IO6LIBRARY_FILES
        TIMER_FILES
        )

pico_enable_stdio_usb(${TARGET_NAME} 0)
pico_enable_stdio_uart(${TARGET_NAME} 1)

pico_add_extra_outputs(${TARGET_NAME})
