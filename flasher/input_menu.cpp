#include <algorithm>
#include <string>

#include <cstring>
#include <cstdio>
#include <cctype>

#include <pico/time.h>
#include <pico/stdlib.h>
#include <pico/stdio.h>

#include "flasher.hpp"
#include "menu.hpp"
#include "input_menu.hpp"

namespace {
    static BuildDate build_date(__DATE__,__TIME__);
}

InputMenu::InputMenu(unsigned max_value_size, ValidInput valid_input,
        const std::string title, const std::string prompt, Menu* base_menu):
    FramedMenu(title,7,std::max({40U,title.size()+6U,max_value_size+prompt.size()+7U})), 
    base_menu_(base_menu), max_value_size_(max_value_size), valid_input_(valid_input), prompt_(prompt)
{
    cls_on_redraw_ = false;
    if(base_menu) { 
        this->set_screen_size(base_menu->screen_height(), base_menu->screen_width());
    }
    timer_interval_us_ = 1000000; // 1Hz
}

InputMenu::InputMenu(unsigned max_value_size, const std::string title,
        const std::string prompt, ValidInput valid_input, Menu* base_menu):
    InputMenu(max_value_size, valid_input, title, prompt, base_menu)
{
    // nothing to see here
}

InputMenu::~InputMenu()
{
    // nothing to see here
}

bool InputMenu::controller_connected(int& return_code)
{
    return true;
}

bool InputMenu::controller_disconnected(int& return_code)
{
    return_code = 0;
    return false;
}

bool InputMenu::process_key_press(int key, int key_count, int& return_code,
    const std::vector<std::string>& escape_sequence_parameters)
{
    switch(key) {
    case KEY_DELETE:
    case 8:   // Ctrl-h or backsapce
    case 127: // Delete
        if(value_.size()) {
            value_.pop_back();
            draw_value();
        }
        break;
    case 21:
        if(value_.size()) {
            value_.clear();
            draw_value();
        }
        break;
    case 3: // Ctrl-c
    case 4: // Ctrl-d
    case 27: // Esc
        curpos(frame_r_+5, frame_c_+ 4);
        puts_center_filled("  CANCELLED  ", frame_w_-6,'X');
        sleep_ms(750);
        return_code = 0;
        return false;
    case '\n':
    case '\r':
        return_code = 1;
        return false;
    default:
        if(value_.size() < max_value_size_ and is_valid(key)) {
            value_.push_back(key);
            draw_value();
        } else {
            putchar_raw(7); // Ctrl-g or BELL
        }
        break;
    }
    return true;
}


bool InputMenu::is_valid(int key)
{
    if(key>127) {
        return false;
    }
    switch(valid_input_) {
    case STRING:
        return isprint(key);
    case FLOAT:        
        if(value_.size()==0)return key=='-' or key=='.' or isdigit(key);
        else if(value_.size()==1) {
            if(value_[0] == '-')return key=='.' or isdigit(key);
            else if(value_[0]=='0')return key=='.';
            else if(value_[0]=='.')return isdigit(key);
            else return isdigit(key) or key=='.';
        } else if(value_.size()==2) {
            if(value_[0] == '-') {
                if(value_[1]=='0')return key=='.';
                else if(value_[1]=='.')return isdigit(key);
                else return isdigit(key) or key=='.';
            } else {
                if(key == '.') {
                    return value_.find('.') == std::string::npos;
                } else {
                    return isdigit(key);
                }
            } 
        } else {
            if(key == '.') {
                return value_.find('.') == std::string::npos;
            } else {
                return isdigit(key);
            }
        }
        return false;
    case POSITIVE_FLOAT:        
        if(value_.size()==0)return key=='.' or isdigit(key);
        else if(value_.size()==1) {
            if(value_[0]=='0')return key=='.';
            else if(value_[0]=='.')return isdigit(key);
            else return isdigit(key) or key=='.';
        } else {
            if(key == '.') {
                return value_.find('.') == std::string::npos;
            } else {
                return isdigit(key);
            }
        }
        return false;
    case INTEGER:
        if(value_.size()==0)return key=='-' or isdigit(key);
        else if(value_.size()==1)return isdigit(key) and (value_[0]!='0' and key!='0');
        else return isdigit(key);
    case NATURAL:
        if(value_.size()==0)return isdigit(key);
        else return isdigit(key) and value_[0]!='0';
    }
    return false;
}

bool InputMenu::process_timer(bool controller_is_connected, int& return_code)
{
    if(controller_is_connected) {
        blink_on_ = !blink_on_;
        if(value_.size() < max_value_size_) {
            curpos(frame_r_+5, frame_c_+prompt_.size()+5+value_.size());
            putchar_raw(blink_on_ ? ' ' : '_');
        }
    }
    return controller_is_connected;
}

void InputMenu::redraw()
{
    if(base_menu_) { base_menu_->redraw(); }
    FramedMenu::redraw();
    curpos(frame_r_+5, frame_c_+4);
    puts_raw_nonl(prompt_);
    putchar_raw(' ');
    for(unsigned i=0;i<value_.size();++i)putchar_raw(value_[i]);
    if(value_.size() < max_value_size_) {
        putchar_raw(blink_on_ ? ' ' : '_');
    }
    for(unsigned i=value_.size()+1;i<max_value_size_;++i)putchar_raw('_');
}

void InputMenu::draw_value()
{
    curpos(frame_r_+5, frame_c_+prompt_.size()+5);
    for(unsigned i=0;i<value_.size();++i)putchar_raw(value_[i]);
    if(value_.size() < max_value_size_) {
        putchar_raw(blink_on_ ? ' ' : '_');
    }
    for(unsigned i=value_.size()+1;i<max_value_size_;++i)putchar_raw('_');
}