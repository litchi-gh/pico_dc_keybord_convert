cmake_minimum_required(VERSION 3.13)
if(POLICY CMP0169)
  cmake_policy(SET CMP0169 OLD)
endif()

if(NOT DEFINED PICO_SDK_PATH)
  if(DEFINED ENV{PICO_SDK_PATH})
    set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
  else()
    message(FATAL_ERROR "PICO_SDK_PATH is not set. Install pico-sdk and set PICO_SDK_PATH.")
  endif()
endif()

include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(pico_dc_keyboard_convert C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

pico_sdk_init()

include(FetchContent)
FetchContent_Declare(
  dreampicoport
  GIT_REPOSITORY https://github.com/OrangeFox86/DreamPicoPort.git
  GIT_TAG d06d75641d5240ae3dc3561bb927563a0217ebc6
  GIT_SHALLOW FALSE
  GIT_SUBMODULES ""
)
FetchContent_GetProperties(dreampicoport)
if(NOT dreampicoport_POPULATED)
  FetchContent_Populate(dreampicoport)
endif()

set(DPP_MAPLE_DIR ${dreampicoport_SOURCE_DIR}/src/hal/MapleBus)

add_library(maple_bus STATIC
  ${DPP_MAPLE_DIR}/MapleBus.cpp
  ${DPP_MAPLE_DIR}/MapleUtils.cpp
)
pico_generate_pio_header(maple_bus ${DPP_MAPLE_DIR}/maple_in.pio)
pico_generate_pio_header(maple_bus ${DPP_MAPLE_DIR}/maple_out.pio)
target_include_directories(maple_bus PUBLIC
  ${dreampicoport_SOURCE_DIR}/inc
  ${dreampicoport_SOURCE_DIR}/src
  ${DPP_MAPLE_DIR}
)
target_link_libraries(maple_bus PUBLIC
  pico_stdlib
  pico_unique_id
  hardware_pio
  hardware_dma
  hardware_gpio
)
target_compile_definitions(maple_bus PUBLIC
  CPU_FREQ_KHZ=133000
  P1_BUS_START_PIN=10
  P1_DIR_PIN=-1
)
# DreamPicoPort's interface uses std::uint64_t but includes <stdint.h> rather
# than <cstdint>; force the standard C++ declaration when used standalone.
target_compile_options(maple_bus PRIVATE
  $<$<COMPILE_LANGUAGE:CXX>:-include>
  $<$<COMPILE_LANGUAGE:CXX>:cstdint>
)

add_executable(dc_keyboard_usb
  src/main.cpp
  src/usb_descriptors.c
)
if(PICO_BOARD STREQUAL "waveshare_rp2040_zero")
  set(DC_KEYBOARD_OUTPUT_NAME dc_keyboard_usb_rp2040_zero)
else()
  set(DC_KEYBOARD_OUTPUT_NAME dc_keyboard_usb)
endif()
set_target_properties(dc_keyboard_usb PROPERTIES
  OUTPUT_NAME ${DC_KEYBOARD_OUTPUT_NAME}
)
target_include_directories(dc_keyboard_usb PRIVATE src)
target_link_libraries(dc_keyboard_usb PRIVATE
  maple_bus
  pico_stdlib
  hardware_clocks
  tinyusb_device
  tinyusb_board
)

pico_enable_stdio_usb(dc_keyboard_usb 0)
pico_enable_stdio_uart(dc_keyboard_usb 0)
pico_add_extra_outputs(dc_keyboard_usb)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${DC_KEYBOARD_OUTPUT_NAME}.uf2 DESTINATION .)
