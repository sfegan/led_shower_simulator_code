#include <string>
#include <vector>

#include <cstring>
#include <cstdio>
#include <cctype>

#include <pico/stdlib.h>

#include "flasher.hpp"
#include "menu.hpp"
#include "main_menu.hpp"
#include "event_generators.hpp"
#include "event_dispatcher.hpp"

std::string BuildDate::latest_build_date = "0000-00-00 00:00:00";

BuildDate::BuildDate(const char* date, const char* time)
{
    // Input date format : "Mmm DD YYYY"
    // Input time format : "HH:MM:SS"
    
    std::string build_date = "0000-00-00 00:00:00";
    build_date[0] = date[7];
    build_date[1] = date[8];
    build_date[2] = date[9];
    build_date[3] = date[10];
    switch(date[0]) {
    case 'J':
        if(date[1] == 'a')build_date[6] = '1';       // Jan
        else if(date[2] == 'n')build_date[6] = '6';  // Jun
        else build_date[6] = '7';                    // Jul
        break;
    case 'F':
        build_date[6] = '2';                         // Feb
        break;
    case 'M':
        if(date[2] == 'r')build_date[6] = '3';       // Mar
        else build_date[6] = '5';                    // May
        break;
    case 'A':
        if(date[1] == 'p')build_date[6] = '4';       // Apr
        else build_date[6] = '8';                    // Aug
        break;
    case 'S':
        build_date[6] = '9';                         // Sep
        break;
    case 'O':
        build_date[6] = '0';                         // Oct
        build_date[5] = '1';
        break;
    case 'N':
        build_date[6] = '1';                         // Nov
        build_date[5] = '1';
        break;
    case 'D':
        build_date[6] = '2';                         // Dec
        build_date[5] = '1';
        break;

    }
    build_date[8] = date[4];
    build_date[9] = date[5];
    build_date[11] = time[0];
    build_date[12] = time[1];
    build_date[14] = time[3];
    build_date[15] = time[4];
    build_date[17] = time[6];
    build_date[18] = time[7];
    if(build_date > latest_build_date) {
        latest_build_date = build_date;
    }
}

namespace {
    static BuildDate build_date(__DATE__,__TIME__);
}

int main()
{
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);

    uint32_t pin_mask =
        (0xFFU << VDAC_BASE_PIN) 
        | (0xFU << ROW_A_BASE_PIN)
        | (0xFU << COL_A_BASE_PIN)
        | (0x1U << DAC_EN_PIN)
        | (0x1U << TRIG_PIN)
        | (0x1U << SPI_CLK_PIN)
        | (0x1U << SPI_DOUT_PIN)
        | (0x1U << SPI_COL_EN_PIN)
        | (0x1U << SPI_ALL_EN_PIN)
        | (0x1U << DAC_WR_PIN)
        | (0x3U << DAC_SEL_BASE_PIN);

    gpio_init_mask(pin_mask);
    gpio_clr_mask(pin_mask);
    gpio_set_dir_out_masked(pin_mask);

    stdio_init_all();

    // EventDispatcher::instance().start_dispatcher();

    MainMenu menu;
    // SingleLEDEventGenerator menu;
    // EventDispatcher::instance().register_event_generator(&menu);
    menu.event_loop();
}
