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

std::string BuildDate::latest_build_date = "00000000000000";

BuildDate::BuildDate(const char* date, const char* time)
{
    std::string timestamp = std::string(date) + " " + std::string(time);
    build_date = timestamp.substr(7,4);
    std::string month = timestamp.substr(0,3);
    if(month == "Jan")build_date += "-01-";
    else if(month == "Feb")build_date += "-02-";
    else if(month == "Mar")build_date += "-03-";
    else if(month == "Apr")build_date += "-04-";
    else if(month == "May")build_date += "-05-";
    else if(month == "Jun")build_date += "-06-";
    else if(month == "Jul")build_date += "-07-";
    else if(month == "Aug")build_date += "-08-";
    else if(month == "Sep")build_date += "-09-";
    else if(month == "Oct")build_date += "-10-";
    else if(month == "Nov")build_date += "-11-";
    else if(month == "Dec")build_date += "-12-";
    build_date += timestamp.substr(4,2);
    build_date += timestamp.substr(11,9);
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
