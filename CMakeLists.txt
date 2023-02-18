cmake_minimum_required(VERSION 3.2)

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

add_executable(libsharp_example main.c)
target_link_libraries(libsharp_example
	libsharp
	hardware_spi
	hardware_gpio
	pico_stdlib
)
