add_executable(flasher
        flasher.cpp menu.cpp
        )

# pull in common dependencies
target_link_libraries(flasher pico_stdlib)
target_compile_definitions(flasher PRIVATE)

# create map/bin/hex file etc.
pico_add_extra_outputs(flasher)

# add url via pico_set_program_url
flasher_auto_set_url(flasher)

pico_enable_stdio_usb(flasher 1)
pico_enable_stdio_uart(flasher 0)

