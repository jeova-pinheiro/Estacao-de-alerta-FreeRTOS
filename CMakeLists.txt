cmake_minimum_required(VERSION 3.13)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PICO_BOARD pico_w CACHE STRING "Board type")
include(pico_sdk_import.cmake)
# set(FREERTOS_KERNEL_PATH "D:/FreeRTOS-Kernel")
set(FREERTOS_KERNEL_PATH "D:/FreeRTOS-Kernel")
include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

project(PiscaLed C CXX ASM)
pico_sdk_init()

include_directories(${CMAKE_SOURCE_DIR}/lib)

add_executable(${PROJECT_NAME}  
        # blink.c 
        Principal.c
        lib/ssd1306.c # Biblioteca para o display OLED
        )

# Generate PIO header
pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/lib/ws2818b.pio)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_enable_stdio_usb(${PROJECT_NAME} 1)

target_link_libraries(${PROJECT_NAME} 
    pico_stdlib 
    FreeRTOS-Kernel         # Kernel do FreeRTOS
    FreeRTOS-Kernel-Heap4   # Gerenciador de memoria

    hardware_pio # Matriz de leds
    hardware_clocks # Matriz de leds
    hardware_i2c # Comunicação do display
    hardware_adc # Joystic
    hardware_pwm # Leds RGB
    hardware_gpio # Entradas GPIO
    # pico_bootsel_via_double_reset # PARA COLOCAR A PLACA NO MODO DE GRAVACAO
    # pico_bootrom # PARA COLOCAR A PLACA NO MODO DE GRAVACAO
)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

pico_add_extra_outputs(${PROJECT_NAME})




