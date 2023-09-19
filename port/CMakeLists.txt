if(${WIZNET_CHIP} STREQUAL W6100)
# io6Library
add_library(IO6LIBRARY_FILES STATIC)

target_sources(IO6LIBRARY_FILES PUBLIC
        ${PORT_DIR}/io6Library/src/w6x00_spi.c
        ${PORT_DIR}/io6Library/src/w6x00_gpio_irq.c
        )

target_include_directories(IO6LIBRARY_FILES PUBLIC
        ${WIZNET6_DIR}/Ethernet
        ${PORT_DIR}/io6Library/inc
        ${PORT_DIR}
        )

target_link_libraries(IO6LIBRARY_FILES PRIVATE
        pico_stdlib
        hardware_spi
        hardware_dma
        hardware_clocks
        )

# lwip6
add_library(LWIP6_FILES STATIC)

target_sources(LWIP6_FILES PUBLIC
        ${PORT_DIR}/lwip6/w6x00_lwip.c
        )

target_include_directories(LWIP6_FILES PUBLIC
        ${WIZNET6_DIR}/Ethernet
        ${PORT_DIR}
        ${PORT_DIR}/lwip6
        )

target_link_libraries(LWIP6_FILES PUBLIC
        ETHERNET6_FILES
        pico_lwip
        pico_lwip_nosys
        )

elseif(${WIZNET_CHIP} STREQUAL W5100S OR ${WIZNET_CHIP} STREQUAL W5500)
# ioLibrary_Driver
add_library(IOLIBRARY_FILES STATIC)

target_sources(IOLIBRARY_FILES PUBLIC
        ${PORT_DIR}/ioLibrary_Driver/src/w5x00_spi.c
        ${PORT_DIR}/ioLibrary_Driver/src/w5x00_gpio_irq.c
        )

target_include_directories(IOLIBRARY_FILES PUBLIC
        ${WIZNET_DIR}/Ethernet
        ${PORT_DIR}/ioLibrary_Driver/inc
        ${PORT_DIR}
        )

target_link_libraries(IOLIBRARY_FILES PRIVATE
        pico_stdlib
        hardware_spi
        hardware_dma
        hardware_clocks
        )

# lwip
add_library(LWIP_FILES STATIC)

target_sources(LWIP_FILES PUBLIC
        ${PORT_DIR}/lwip/w5x00_lwip.c
        )

target_include_directories(LWIP_FILES PUBLIC
        ${WIZNET_DIR}/Ethernet
        ${PORT_DIR}
        ${PORT_DIR}/lwip
        )

target_link_libraries(LWIP_FILES PUBLIC
        ETHERNET_FILES
        pico_lwip
        pico_lwip_nosys
        )
elseif(${WIZNET_CHIP} STREQUAL ENC28J60)
#ENC28J60
add_library(ENC28J60_FILES STATIC)

target_sources(ENC28J60_FILES PUBLIC
        #${PORT_DIR}/enc28j60/enc28j60_lwip.c
        )

target_include_directories(ENC28J60_FILES PUBLIC
        ${PORT_DIR}
        ${PORT_DIR}/enc28j60
        )

target_link_libraries(ENC28J60_FILES PUBLIC
        pico_lwip
        pico_lwip_nosys
        )
endif()

# timer
add_library(TIMER_FILES STATIC)

target_sources(TIMER_FILES PUBLIC
        ${PORT_DIR}/timer/timer.c
        )

target_include_directories(TIMER_FILES PUBLIC
        ${PORT_DIR}/timer
        )

target_link_libraries(TIMER_FILES PRIVATE
        pico_stdlib      
        )
