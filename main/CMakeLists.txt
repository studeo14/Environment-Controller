idf_component_register(SRCS "blink_example_main.c" "led/led.c" "wifi/wifi.c" "periphs/periphs.c" "periphs/sht31.c" "periphs/pcf.c" "mqtt.c" "screen.c"
                       INCLUDE_DIRS ".")
target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")