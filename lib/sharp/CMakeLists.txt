add_library(libsharp INTERFACE)
target_sources(libsharp INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/sharp.c
)
target_include_directories(libsharp INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)
target_link_libraries(libsharp INTERFACE
    hardware_spi
    hardware_gpio
    pico_stdlib
)