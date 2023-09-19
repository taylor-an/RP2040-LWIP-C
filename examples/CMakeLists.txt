if(${WIZNET_CHIP} STREQUAL W6100)
add_subdirectory(lwiperf6)
add_subdirectory(iperf)

elseif(${WIZNET_CHIP} STREQUAL W5100S OR ${WIZNET_CHIP} STREQUAL W5500)
add_subdirectory(dhcp_dns)
add_subdirectory(loopback)
add_subdirectory(lwiperf)

elseif(${WIZNET_CHIP} STREQUAL ENC28J60)
add_subdirectory(lwiperf-enc28j60)

endif()
