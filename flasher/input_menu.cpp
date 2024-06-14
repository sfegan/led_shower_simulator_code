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

InplaceInputMenu::InplaceInputMenu(int r, int c, unsigned max_value_size, ValidInput valid_input, 
        bool do_highlight, Menu* base_menu, RowAndColumnGetter* row_col_getter):
    Menu(), base_menu_(base_menu), row_col_getter_(row_col_getter),
    r_(r), c_(c), max_value_size_(max_value_size), valid_input_(valid_input),
    do_highlight_(do_highlight)
{
    timer_interval_us_ = 1000000; // 1Hz
}

InplaceInputMenu::~InplaceInputMenu()
{
    // nothing to see here
}

bool InplaceInputMenu::controller_connected(int& return_code)
{
    return true;
}

bool InplaceInputMenu::controller_disconnected(int& return_code)
{
    return_code = 0;
    return false;
}

bool InplaceInputMenu::process_key_press(int key, int key_count, int& return_code,
    const std::vector<std::string>& escape_sequence_parameters)
{
    switch(key) {
    case KEY_DELETE:
    case 8:   // Ctrl-h or backspace
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

bool InplaceInputMenu::is_valid(int key)
{
    if(key>127) {
        return false;
    }
    switch(valid_input_) {
    case VI_STRING:
        return isprint(key);
    case VI_FLOAT:        
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
    case VI_POSITIVE_FLOAT:        
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
    case VI_INTEGER:
        if(value_.size()==0)return key=='-' or isdigit(key);
        else if(value_.size()==1)return isdigit(key) and (value_[0]!='0' and key!='0');
        else return isdigit(key);
    case VI_NATURAL:
        if(value_.size()==0)return isdigit(key);
        else return isdigit(key) and value_[0]!='0';
    }
    return false;
}

bool InplaceInputMenu::process_timer(bool controller_is_connected, int& return_code)
{
    if(controller_is_connected) {
        blink_on_ = !blink_on_;
        if(value_.size() < max_value_size_) {
            curpos(r_+1, c_+1+value_.size());
            if(do_highlight_) {
                save_cursor();
                highlight();
            }
            putchar_raw(blink_on_ ? ' ' : '_');
            if(do_highlight_) {
                restore_cursor();
            }
        }
    }
    return controller_is_connected;
}

void InplaceInputMenu::redraw()
{
    if(base_menu_ and not first_redraw_) { base_menu_->redraw(); }
    first_redraw_ = false;
    if(row_col_getter_) { row_col_getter_->get_row_and_column(r_,c_); }
    draw_value();
}

void InplaceInputMenu::draw_value()
{
    curpos(r_+1, c_+1);
    if(do_highlight_) {
        save_cursor();
        highlight();
    }
    for(unsigned i=0;i<value_.size();++i)putchar_raw(value_[i]);
    if(value_.size() < max_value_size_) {
        putchar_raw(blink_on_ ? ' ' : '_');
    }
    for(unsigned i=value_.size()+1;i<max_value_size_;++i)putchar_raw('_');
    if(do_highlight_) {
        restore_cursor();
    }
}

void InplaceInputMenu::cancelled()
{
    curpos(r_+1, c_+1);
    if(do_highlight_) {
        save_cursor();
        highlight();
    }
    for(unsigned i=0;i<max_value_size_;++i)putchar_raw('X');
    if(do_highlight_) {
        restore_cursor();
    }
    sleep_ms(750);
}

InputMenu::InputMenu(unsigned max_value_size, ValidInput valid_input,
        const std::string title, const std::string prompt, Menu* base_menu):
    FramedMenu(title,7,std::max({40U,title.size()+6U,max_value_size+prompt.size()+7U})), 
    iim_(frame_r_+5, frame_c_+prompt_.size()+5, max_value_size, valid_input, false, nullptr),
    base_menu_(base_menu), prompt_(prompt)
{
    cls_on_redraw_ = false;
    timer_interval_us_ = iim_.timer_interval_us();
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
    return iim_.controller_connected(return_code);
}

bool InputMenu::controller_disconnected(int& return_code)
{
    return iim_.controller_disconnected(return_code);
}

bool InputMenu::process_key_press(int key, int key_count, int& return_code,
    const std::vector<std::string>& escape_sequence_parameters)
{
    return iim_.process_key_press(key, key_count, return_code, escape_sequence_parameters);
}

bool InputMenu::process_timer(bool controller_is_connected, int& return_code)
{
    return iim_.process_timer(controller_is_connected, return_code);
}

void InputMenu::redraw()
{
    if(base_menu_ and not first_redraw_) { base_menu_->redraw(); }
    first_redraw_ = false;
    FramedMenu::redraw();
    curpos(frame_r_+5, frame_c_+4);
    puts_raw_nonl(prompt_);
    iim_.redraw();
}

const std::string InputMenu::get_value() const
{
    return iim_.get_value();
}

void InputMenu::cancelled()
{
    curpos(frame_r_+5, frame_c_+ 4);
    puts_center_filled("  CANCELLED  ", frame_w_-6,'X');
    sleep_ms(750);
}
