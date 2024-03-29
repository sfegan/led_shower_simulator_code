set(LED_SHOWER_SIMULATOR_CODE_PATH_URL_BASE "https://github.com/sfegan/led_shower_simulator_code/tree/HEAD")
macro(flasher_auto_set_url TARGET)
    file(RELATIVE_PATH URL_REL_PATH "${LED_SHOWER_SIMULATOR_CODE_PATH}" "${CMAKE_CURRENT_LIST_DIR}")
    pico_set_program_url(${TARGET} "${LED_SHOWER_SIMULATOR_CODE_PATH_URL_BASE}/${URL_REL_PATH}")
endmacro()