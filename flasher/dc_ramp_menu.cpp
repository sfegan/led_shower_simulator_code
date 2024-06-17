#include "flasher.hpp"
#include "menu.hpp"
#include "input_menu.hpp"
#include "dc_ramp_menu.hpp"

DCRampMenu::DCRampMenu() : 
    SimpleItemValueMenu(make_menu_items(), "DC Ramp menu") 
{
    sync_values();
}

void DCRampMenu::sync_values()
{
    unsigned all_gpio = gpio_get_all();
    ar_      = (all_gpio >> ROW_A_BASE_PIN) & 0x00000F;
    ac_      = (all_gpio >> COL_A_BASE_PIN) & 0x00000F;
    set_scale_value(false);
    set_rc_value(false);
}

void DCRampMenu::set_rc_value(bool draw) 
{ 
    menu_items_[MIP_ROWCOL].value = std::string(1, char('A' + ar_)) 
        + std::to_string(ac_); 
    if(draw)draw_item_value(MIP_ROWCOL);
}

void DCRampMenu::set_scale_value(bool draw) 
{ 
    menu_items_[MIP_SCALE_DAC].value = std::to_string(scale_); 
    if(draw)draw_item_value(MIP_SCALE_DAC);
}

void DCRampMenu::set_offset_value(bool draw) 
{ 
    menu_items_[MIP_TRIM_DAC].value = std::to_string(offset_); 
    if(draw)draw_item_value(MIP_TRIM_DAC);
}

std::vector<SimpleItemValueMenu::MenuItem> DCRampMenu::make_menu_items() 
{
    std::vector<SimpleItemValueMenu::MenuItem> menu_items(MIP_NUM_ITEMS);
    menu_items.at(MIP_ROWCOL)      = {"Cursors : Change column & row", 3, "A1"};
    menu_items.at(MIP_SCALE_DAC)   = {"S/s     : Ramp scale", 3, "0"};
    menu_items.at(MIP_TRIM_DAC)    = {"O/o     : Ramp offset", 3, "0"};
    menu_items.at(MIP_EXIT)        = {"q/Q     : Exit menu", 0, ""};
    return menu_items;
}

bool DCRampMenu::controller_connected(int& return_code)
{
    return_code = 0;
    return true;
}

bool DCRampMenu::controller_disconnected(int& return_code)
{
    return_code = 0;
    return true;
}

bool DCRampMenu::process_key_press(int key, int key_count, int& return_code,
    const std::vector<std::string>& escape_sequence_parameters)
{
    switch(key) {
    case 'q':
    case 'Q':
        return_code = 0;
        return false;
    case KEY_UP:
        ar_ = std::max(ar_-1, 0);
        set_rc_value();
        break;
    case KEY_DOWN:
        ar_ = std::min(ar_+1, 15);
        set_rc_value();
        break;
    case KEY_LEFT:
        ac_ = std::max(ac_-1, 0);
        set_rc_value();
        break;
    case KEY_RIGHT:
        ac_ = std::min(ac_+1, 15);
        set_rc_value();
        break;
    case KEY_PAGE_UP:
        ar_ = 0;
        set_rc_value();
        break;
    case KEY_PAGE_DOWN:
        ar_ = 15;
        set_rc_value();
        break;
    case KEY_HOME:
        ac_ = 0;
        set_rc_value();
        break;
    case KEY_END:
        ac_ = 15;
        set_rc_value();
        break;
    case 'S':
    case 's':
        {
            SimpleItemValueRowAndColumnGetter rc_getter(this, MIP_SCALE_DAC);
            InplaceInputMenu input(item_r_+MIP_SCALE_DAC*item_dr_, val_c_, 3, VI_NATURAL, 
                true, this, &rc_getter);
            if(input.event_loop()==1 and input.get_value().size()!=0) {
                int val = std::stoi(input.get_value());
                if(val>=0 and val<=255) {
                    scale_ = val;
                } else {
                    input.cancelled();
                }
            } else {
                input.cancelled();
            }
            set_scale_value(true);
        }
        break;
    case 'O':
    case 'o':
        {
            SimpleItemValueRowAndColumnGetter rc_getter(this, MIP_TRIM_DAC);
            InplaceInputMenu input(item_r_+MIP_TRIM_DAC*item_dr_, val_c_, 3, VI_NATURAL, 
                true, this, &rc_getter);
            if(input.event_loop()==1 and input.get_value().size()!=0) {
                int val = std::stoi(input.get_value());
                if(val>=0 and val<=255) {
                    offset_ = val;
                } else {
                    input.cancelled();
                }
            } else {
                input.cancelled();
            }
            set_offset_value(true);
        }
        break;
    }

    return true;
}

bool DCRampMenu::process_timer(bool controller_is_connected, int& return_code)
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