#include <string>
#include <vector>

#include <cstring>
#include <cstdio>
#include <cctype>

#include <pico/stdlib.h>

#include "flasher.hpp"
#include "menu.hpp"
#include "engineering_menu.hpp"
#include "event_generators.hpp"
#include "event_dispatcher.hpp"

class KeypressMenu: public Menu {
public:
    virtual ~KeypressMenu() { }
    void redraw() override;
    bool controller_connected(int& return_code) final;
    bool controller_disconnected(int& return_code) final;
    bool process_key_press(int key, int key_count, int& return_code,
        const std::vector<std::string>& escape_sequence_parameters) final;
    bool process_timer(bool controller_is_connected, int& return_code) final;
};

void KeypressMenu::redraw()
{
    cls();
    curpos(1,1);
    puts("Type some keys (terminate with Ctrl-D)");
}

bool KeypressMenu::controller_connected(int& return_code)
{
    return_code = 0;
    return true;
}

bool KeypressMenu::controller_disconnected(int& return_code)
{
    return_code = 0;
    return true;
}

bool KeypressMenu::process_key_press(int key, int key_count, int& return_code,
    const std::vector<std::string>& escape_sequence_parameters)
{
    char buffer[80];
    sprintf(buffer, "%c %d \\%o %d",(key<256 and isprint(key))?key:' ',key,key,key_count);
    if(escape_sequence_parameters.empty()) {
        puts(buffer);    
    } else {
        puts_raw_nonl(buffer);
        puts_raw_nonl(" (");
        for(unsigned i=0; i<escape_sequence_parameters.size(); ++i) {
            if(i!=0)puts_raw_nonl(", ");
            puts_raw_nonl(escape_sequence_parameters[i]);
        }
        puts(")");
    }
    return_code = 0;
    if(key == '\003') {
        send_request_screen_size();
    } else if(key == '\010') {
        hide_cursor();
    } else if(key == '\011') {
        show_cursor();
    }
    return key != '\004';
}

bool KeypressMenu::process_timer(bool controller_is_connected, int& return_code)
{
    return_code = 0;
    return true;
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

    EngineeringMenu menu;
    // SingleLEDEventGenerator menu;
    // EventDispatcher::instance().register_event_generator(&menu);
    menu.event_loop();
}
