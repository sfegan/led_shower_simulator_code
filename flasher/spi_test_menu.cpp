#include "flasher.hpp"
#include "menu.hpp"
#include "input_menu.hpp"
#include "spi_test_menu.hpp"

namespace {
    static BuildDate build_date(__DATE__,__TIME__);
}

SPItestMenu::SPItestMenu() : 
    SimpleItemValueMenu(make_menu_items(), "SPI test Menu") 
{
    sync_values();
}

void SPItestMenu::sync_values()
{
    unsigned all_gpio = gpio_get_all();
    vdac_    = (all_gpio >> VDAC_BASE_PIN)  & 0x0000FF;
    ar_      = (all_gpio >> ROW_A_BASE_PIN) & 0x00000F;
    ac_      = (all_gpio >> COL_A_BASE_PIN) & 0x00000F;
}

void SPItestMenu::set_rc_value(bool draw) 
{ 
    rc_to_value_string(menu_items_[MIP_ROWCOL].value, ar_, ac_);
    if(draw)draw_item_value(MIP_ROWCOL);
}

void SPItestMenu::set_delay_value(bool draw)
{ 
    menu_items_[MIP_DELAY].value = std::to_string(delay_); 
    if(draw)draw_item_value(MIP_DELAY);
}

void SPItestMenu::program_delay()
{
    mask_ = 128;
    int i;
    gpio_put(SPI_CLK_PIN, 1);
    gpio_put(SPI_COL_EN_PIN, 1);
    delay();
    for(i = 7; i >= 0; i -= 1) {
        gpio_put(SPI_DOUT_PIN, delay_ & mask_ ? 1 : 0);
        gpio_put(SPI_CLK_PIN, 0);
        delay();
        gpio_put(SPI_CLK_PIN, 1);
        delay();
        mask_ = mask_ >> 1;
    }
    gpio_put(SPI_COL_EN_PIN, 0);
    delay();
    gpio_put(SPI_CLK_PIN, 0);
    delay();
    gpio_put(SPI_DOUT_PIN, 0);
    menu_delay_ = false;
    set_menu_delay_value();
}

void SPItestMenu::set_menu_delay_value(bool draw) 
{ 
    menu_items_[MIP_MENU_DELAY].value = menu_delay_ ? ">ENABLE<" : "disable"; 
    menu_items_[MIP_MENU_DELAY].value_style = menu_delay_ ? ANSI_INVERT : "";
    if(draw)draw_item_value(MIP_MENU_DELAY);
}

void SPItestMenu::delay()
{
    sleep_ms(500);
}

std::vector<SimpleItemValueMenu::MenuItem> SPItestMenu::make_menu_items() 
{
    std::vector<SimpleItemValueMenu::MenuItem> menu_items(MIP_NUM_ITEMS);
    menu_items.at(MIP_ROWCOL)     = {"Cursors : Change column & row", 3, "A0"};
    menu_items.at(MIP_DELAY)      = {"</d/>   : Delay", 3, "0"};
    menu_items.at(MIP_MENU_DELAY) = {"P       : Program delay", 8, "disable"};
    menu_items.at(MIP_EXIT)       = {"Q       : Exit menu", 0, ""};
    return menu_items;
}

bool SPItestMenu::controller_connected(int& return_code)
{
    return_code = 0;
    return true;
}

bool SPItestMenu::controller_disconnected(int& return_code)
{
    return_code = 0;
    return true;
}

bool SPItestMenu::process_key_press(int key, int key_count, int& return_code,
    const std::vector<std::string>& escape_sequence_parameters, absolute_time_t& next_timer)
{
    if(process_rc_keys(ar_, ac_, key, key_count)) {
        set_rc_value();
        return true;
    }

    switch (key) {
        case '<':
            decrease_value_in_range(delay_, 0, (key_count >= 15 ? 5 : 1), key_count==1);
            set_delay_value();
            break;
        case '>':
            increase_value_in_range(delay_, 255, (key_count >= 15 ? 5 : 1), key_count==1);
            set_delay_value();
            break;
        case 'D':
        case 'd':
            {
                SimpleItemValueRowAndColumnGetter rc_getter(this, MIP_DELAY);
                InplaceInputMenu input(rc_getter, 3, VI_NATURAL, true, this);
                if(input.event_loop()==1 and input.get_value().size()!=0) {
                    int val = std::stoi(input.get_value());
                    if(val>=0 and val<=255) {
                        delay_ = val;
                    } else {
                        input.cancelled();
                    }
                } else {
                    input.cancelled();
                }
                set_delay_value(true);
            }
            break;
        case 'P':
            menu_delay_ = true;
            set_menu_delay_value();
            program_delay();
            break;
        break;
        case 'q':
        case 'Q':
            return_code = 0;
            return false;

        default:
            beep();
    }
    return true;
}

bool SPItestMenu::process_timer(bool controller_is_connected, int& return_code, absolute_time_t& next_timer)
{
    heartbeat_timer_count_ += 1;
    if(heartbeat_timer_count_ == 100) {
        if(controller_is_connected) {
            set_heartbeat(!heartbeat_);
        }
        heartbeat_timer_count_ = 0;
    }
    return true;
}