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
    vdac_ = (all_gpio >> VDAC_BASE_PIN)  & 0x0000FF;
    ar_   = (all_gpio >> ROW_A_BASE_PIN) & 0x00000F;
    ac_   = (all_gpio >> COL_A_BASE_PIN) & 0x00000F;
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

void DCRampMenu::set_ramp_up_time_value(bool draw) 
{ 
    menu_items_[MIP_RAMP_UP].value = std::to_string(ramp_up_time_); 
    if(draw)draw_item_value(MIP_RAMP_UP);
}

void DCRampMenu::set_ramp_hold_time_value(bool draw) 
{ 
    menu_items_[MIP_RAMP_HOLD].value = std::to_string(ramp_hold_time_); 
    if(draw)draw_item_value(MIP_RAMP_HOLD);
}

void DCRampMenu::set_ramp_down_time_value(bool draw) 
{ 
    menu_items_[MIP_RAMP_DOWN].value = std::to_string(ramp_down_time_); 
    if(draw)draw_item_value(MIP_RAMP_DOWN);
}

void DCRampMenu::set_enable_ramp_value(bool draw) 
{ 
    menu_items_[MIP_ENABLE_RAMP].value = enable_ramp_ ? "disable" : ">ENABLE<"; 
    menu_items_[MIP_ENABLE_RAMP].value_style = !enable_ramp_ ? ANSI_INVERT : "";
    if(draw)draw_item_value(MIP_ENABLE_RAMP);
}

void DCRampMenu::set_phase_value(bool draw) 
{
    static const char* name[]= {"NONE", "UP", "HOLD", "DOWN"};
    menu_items_[MIP_PHASE].value = name[phase_]; 
    if(draw)draw_item_value(MIP_PHASE);
}

void DCRampMenu::set_time_value(bool draw) 
{ 
    menu_items_[MIP_TIME].value = std::to_string(time_/100); 
    if(draw)draw_item_value(MIP_TIME);
}

void DCRampMenu::set_vdac_value(bool draw) 
{ 
    menu_items_[MIP_VDAC].value = std::to_string(vdac_); 
    if(draw)draw_item_value(MIP_VDAC);
}

void DCRampMenu::set_clear_ramp_value(bool draw) 
{ 
    menu_items_[MIP_CLEAR_RAMP].value = clear_ramp_ ? ">CLEAR<" : "CLEAR"; 
    menu_items_[MIP_CLEAR_RAMP].value_style = clear_ramp_ ? ANSI_INVERT : "";
    if(draw)draw_item_value(MIP_CLEAR_RAMP); 
}

std::vector<SimpleItemValueMenu::MenuItem> DCRampMenu::make_menu_items() 
{
    std::vector<SimpleItemValueMenu::MenuItem> menu_items(MIP_NUM_ITEMS);
    menu_items.at(MIP_ROWCOL)      = {"Cursors : Change column & row", 3, "A1"};
    menu_items.at(MIP_SCALE_DAC)   = {"S/s     : Ramp scale", 3, "0"};
    menu_items.at(MIP_TRIM_DAC)    = {"O/o     : Ramp offset", 3, "0"};
    menu_items.at(MIP_RAMP_UP)     = {"U/u     : Ramp-up time", 5, "0"};
    menu_items.at(MIP_RAMP_HOLD)   = {"H/h     : Hold time", 5, "0"};
    menu_items.at(MIP_RAMP_DOWN)   = {"D/d     : Ramp-down time", 5, "0"};
    menu_items.at(MIP_ENABLE_RAMP) = {"E       : Enable / disable ramp", 8, "disable"};
    menu_items.at(MIP_CLEAR_RAMP)  = {"C       : Interrupt or/and clear ramp", 7, "CLEAR"};
    menu_items.at(MIP_PHASE)       = {"        : indicates the phase", 4, "NONE"};
    menu_items.at(MIP_TIME)        = {"        : indicates the time", 5, "0"};
    menu_items.at(MIP_VDAC)        = {"        : indicates the intensity", 3, "0"};
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
            InplaceInputMenu input(rc_getter, 3, VI_NATURAL, true, this);
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
            InplaceInputMenu input(rc_getter, 3, VI_NATURAL, true, this);
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
    case 'U':
    case 'u':
        {
        SimpleItemValueRowAndColumnGetter rc_getter(this, MIP_RAMP_UP);
            InplaceInputMenu input(rc_getter, 5, VI_POSITIVE_FLOAT, true, this);
            if(input.event_loop()==1 and input.get_value().size()!=0) {
                float val = std::stof(input.get_value());
                if(val>=0.1 and val<=10) {
                    ramp_up_time_ = val;
                } else {
                    input.cancelled();
                }
            } else {
                input.cancelled();
            }
            set_ramp_up_time_value(true);
        }
        break;
    case 'H':
    case 'h':
        {
        SimpleItemValueRowAndColumnGetter rc_getter(this, MIP_RAMP_HOLD);
            InplaceInputMenu input(rc_getter, 5, VI_POSITIVE_FLOAT, true, this);
            if(input.event_loop()==1 and input.get_value().size()!=0) {
                float val = std::stof(input.get_value());
                if(val>=0 and val<=30) {
                    ramp_hold_time_ = val;
                } else {
                    input.cancelled();
                }
            } else {
                input.cancelled();
            }
            set_ramp_hold_time_value(true);
        }
        break;
    case 'D':
    case 'd':
        {
        SimpleItemValueRowAndColumnGetter rc_getter(this, MIP_RAMP_DOWN);
            InplaceInputMenu input(rc_getter, 5, VI_POSITIVE_FLOAT, true, this);
            if(input.event_loop()==1 and input.get_value().size()!=0) {
                float val = std::stof(input.get_value());
                if(val>=0.1 and val<=10) {
                    ramp_down_time_ = val;
                } else {
                    input.cancelled();
                }
            } else {
                input.cancelled();
            }
            set_ramp_down_time_value(true);
        }
        break;
    case 'E':
        enable_ramp_ = !enable_ramp_;
        set_enable_ramp_value();
        break;
    case 'C':
        if (!enable_ramp_){
            enable_ramp_ = !enable_ramp_;
            set_enable_ramp_value();
        }
        time_ = 0;
        phase_ = 0;
        vdac_ = 0;
        gpio_put_masked(0x0000FF << VDAC_BASE_PIN, vdac_ << VDAC_BASE_PIN);
        set_time_value();
        set_phase_value();
        set_vdac_value();
        clear_ramp_ = !clear_ramp_;
        set_clear_ramp_value();
        sleep_ms(100);
        clear_ramp_ = !clear_ramp_;
        set_clear_ramp_value();
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

    if (!enable_ramp_){
        time_ += 1;
        set_time_value();
        if (time_ < ramp_up_time_*100){
            phase_ = 1;
            set_phase_value();
            vdac_ = (255 / (ramp_up_time_*100)) * time_;
            gpio_put_masked(0x0000FF << VDAC_BASE_PIN, vdac_ << VDAC_BASE_PIN);
            set_vdac_value();
        }

        if (ramp_up_time_*100 <= time_ and time_ <= ramp_up_time_*100 + ramp_hold_time_*100){
            phase_ = 2;
            set_phase_value();
            vdac_ = 255;
            gpio_put_masked(0x0000FF << VDAC_BASE_PIN, vdac_ << VDAC_BASE_PIN);
            set_vdac_value();
        }

        if (ramp_up_time_*100 + ramp_hold_time_*100 < time_ and ramp_up_time_*100 + ramp_hold_time_*100 + ramp_down_time_*100 > time_){
            phase_ = 3;
            set_phase_value();
            vdac_ = (-255 / (ramp_down_time_*100)) * (time_ - (ramp_up_time_*100 + ramp_hold_time_*100)) + 255;
            gpio_put_masked(0x0000FF << VDAC_BASE_PIN, vdac_ << VDAC_BASE_PIN);
            set_vdac_value();
        }

        if (time_ > ramp_up_time_*100 + ramp_hold_time_*100 + ramp_down_time_*100){
            enable_ramp_ = !enable_ramp_;
            phase_ = 0;
            time_ = 0;
            vdac_ = 0;
            gpio_put_masked(0x0000FF << VDAC_BASE_PIN, vdac_ << VDAC_BASE_PIN);
            set_enable_ramp_value();
            set_phase_value();
            set_time_value();
            set_vdac_value();
        }
    }
    return true;
}