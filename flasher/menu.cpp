#include <string>
#include <vector>
#include <algorithm>

#include <cstring>
#include <cstdio>
#include <cctype>

#include <pico/time.h>
#include <pico/stdlib.h>
#include <pico/stdio.h>
#include <hardware/watchdog.h>

#include "flasher.hpp"
#include "menu.hpp"
#include "reboot_menu.hpp"

namespace {
    static BuildDate build_date(__DATE__,__TIME__);
}

Menu::~Menu() 
{ 
    // nothing to see here
}

int Menu::puts_raw_nonl(const char* s) 
{
    for (size_t i = 0; s[i]; ++i) {
        if (putchar_raw(s[i]) == EOF) return EOF;
    }
    return 0;
}

int Menu::puts_raw_nonl(const char* s, size_t maxchars, bool fill) 
{
    for (size_t i = 0; s[i] && maxchars; ++i, --maxchars) {
        if (putchar_raw(s[i]) == EOF) return EOF;
    }
    if(fill && maxchars) {
        while(maxchars--) {
            if (putchar_raw(' ') == EOF) return EOF;
        }
    }
    return 0;
}

int Menu::puts_raw_nonl(const std::string& s) {
    for (size_t i=0; i<s.size(); ++i) {
        if (putchar_raw(s[i]) == EOF) return EOF;
    }
    return 0;
}

int Menu::puts_raw_nonl(const std::string& s, size_t maxchars, bool fill) 
{
    size_t schars = std::min(maxchars, s.size());
    for (size_t i=0; i<schars; ++i, --maxchars) {
        if (putchar_raw(s[i]) == EOF) return EOF;
    }
    if(fill && maxchars) {
        while(maxchars--) {
            if (putchar_raw(' ') == EOF) return EOF;
        }
    }
    return 0;
}

int Menu::puts_formatted(const std::string& s, const std::string& format, 
    size_t maxchars, bool fill)
{
    size_t schars = std::min(maxchars, s.size());
    if(!format.empty()) {
        if(puts_raw_nonl("\0337") == EOF or puts_raw_nonl(format) == EOF)
            return EOF;
    }
    for (size_t i=0; i<schars; ++i, --maxchars) {
        if (putchar_raw(s[i]) == EOF) return EOF;
    }
    if(fill && maxchars) {
        while(maxchars--) {
            if (putchar_raw(' ') == EOF) return EOF;
        }
    }
    if(!format.empty()) {
        if (puts_raw_nonl("\0338") == EOF)return EOF;
    }
    return 0;
}

int Menu::puts_center_filled(const std::string& s, size_t maxchars, char fill_char)
{
    size_t schars = std::min(maxchars, s.size());
    size_t fchars = (maxchars-schars)/2;
    for (size_t i=0; i<fchars; ++i, --maxchars) {
        if (putchar_raw(fill_char) == EOF) return EOF;
    }
    for (size_t i=0; i<schars; ++i, --maxchars) {
        if (putchar_raw(s[i]) == EOF) return EOF;
    }
    while(maxchars--) {
        if (putchar_raw(fill_char) == EOF) return EOF;
    }
    return 0;
}


void Menu::cls() 
{ 
    puts_raw_nonl("\033[2J"); 
}

void Menu::show_cursor() 
{ 
    puts_raw_nonl("\033[?25h"); //\0337p"); 
}

void Menu::hide_cursor() 
{ 
    puts_raw_nonl("\033[?25l"); //\0336p"); 
}

void Menu::curpos(int r, int c) 
{ 
    char buffer[80]; 
    sprintf(buffer,"\033[%d;%dH",r,c); 
    puts_raw_nonl(buffer);
}

void Menu::save_cursor()
{
    puts_raw_nonl("\0337");
}

void Menu::restore_cursor()
{
    puts_raw_nonl("\0338");
}

void Menu::highlight()
{
    puts_raw_nonl(ANSI_INVERT);
}

void Menu::reset_colors()
{
    puts_raw_nonl("\033[m");
}

void Menu::send_request_screen_size() 
{
    puts_raw_nonl("\0337\033[999;999H\033[6n\0338");
}

void Menu::draw_box(int fh, int fw, int fr, int fc) {
    curpos(fr+1,fc+1);
    putchar_raw('+');
    for(int ic=2;ic<fw;++ic)putchar_raw('-');
    putchar_raw('+');
    for(int ir=2;ir<fh;++ir) {
        curpos(fr+ir,fc+1);
        putchar_raw('|');
        for(int ic=2;ic<fw;++ic)putchar_raw(' ');
        putchar_raw('|');
    }
    curpos(fr+fh,fc+1);
    putchar_raw('+');
    for(int ic=2;ic<fw;++ic)putchar_raw('-');
    putchar_raw('+');
}

bool Menu::draw_title(const std::string& title, int fh, int fw, int fr, int fc,
     const std::string& title_style) 
{
    char buffer[80];
    if(fh<5 || fw<5)return false;
    int tw = std::min(int(title.size()), fw-4);
    int tc = fc + (fw-tw)/2;
    sprintf(buffer,"\033[%d;%dH",fr+3,tc+1);
    puts_raw_nonl(buffer);
    if(!title_style.empty())puts_raw_nonl(title_style);
    puts_raw_nonl(title, tw);
    if(!title_style.empty())puts_raw_nonl("\033[0m");
    return tw == int(title.size());
}

bool Menu::draw_heart(bool on, int fh, int fw, int fr, int fc)
{
    if(fh<2 || fw<6)return false;
    curpos(fr+fh+1, fc+fw+1-4);
    if(on)puts_raw_nonl("<3");
    else puts_raw_nonl("--");
    return true;
}

int Menu::event_loop(bool enable_escape_sequences, bool enable_reboot)
{
    static const int64_t multi_keypress_timeout = 100000; /* 100ms */
    absolute_time_t next_timer = delayed_by_us(get_absolute_time(), timer_interval_us_);
    uint64_t timer_delay = timer_interval_us_;
    int return_code = 0;
    bool was_connected = false;
    int last_key = -1;
    absolute_time_t last_key_time = get_absolute_time();
    int key_count = 0;
    bool sent_request_window_size = false;
    std::string escape_sequence;
    std::vector<std::string> escape_sequence_parameters;
    while(true) {
        if(stdio_usb_connected()) {
            if(!was_connected) {
                last_key_time = get_absolute_time();
                if(!this->controller_connected(return_code)) {
                    return return_code;
                }
                if(enable_escape_sequences) {
                    this->send_request_screen_size();
                    sent_request_window_size = true;
                } else {
                    this->redraw();
                }
            }
            was_connected = true;
            int key = getchar_timeout_us(timer_delay);
            absolute_time_t key_time = get_absolute_time();
            if(absolute_time_diff_us(last_key_time, key_time)>multi_keypress_timeout) {
                last_key = -1;
                key_count = 0;
                if(sent_request_window_size) {
                    this->redraw();
                    sent_request_window_size = false;
                }
                if(!escape_sequence.empty()) {
                    for(auto k : escape_sequence) {
                        if(!this->process_key_press(k, 1, return_code, {})) {
                            return return_code;
                        }
                    }
                    escape_sequence.clear();
                    escape_sequence_parameters.clear();
                }
            }
            if(key >= 0) {
                last_key_time = key_time;
                if(!escape_sequence.empty()) {
                    int escaped_key =
                        decode_partial_escape_sequence(key, escape_sequence,    
                            escape_sequence_parameters);
                    switch(escaped_key) {
                    case FAILED_ESCAPE_SEQUENCE:
                        for(auto k : escape_sequence) {
                            if(!this->process_key_press(k, 1, return_code, {})) {
                                return return_code;
                            }
                        }
                        last_key = -1;
                        key_count = 0;
                        escape_sequence.clear();
                        escape_sequence_parameters.clear();
                        break;
                    case INCOMPLETE_ESCAPE_SEQUENCE:
                        break;
                    case UNSUPPORTED_ESCAPE_SEQUENCE:
                        last_key = -1;
                        key_count = 0;
                        escape_sequence.clear();
                        escape_sequence_parameters.clear();
                        break;
                    case CURSOR_POSITION_REPORT:
                        if(sent_request_window_size) {
                            if(escape_sequence_parameters.size() == 2) {
                                int h = std::stoi(escape_sequence_parameters[0]);
                                int w = std::stoi(escape_sequence_parameters[1]);
                                this->set_screen_size(h,w);
                            }                                
                            this->redraw();
                            sent_request_window_size = false;
                        } else {
                            last_key = -1;
                            key_count = 0;
                            if(!this->process_key_press(escaped_key, 1, 
                                return_code, escape_sequence_parameters))
                            {
                                return return_code;
                            }
                        }
                        escape_sequence.clear();
                        escape_sequence_parameters.clear();
                        break;
                    default:
                        if(sent_request_window_size) {
                            this->redraw();
                            sent_request_window_size = false;
                        }
                        if(escaped_key == last_key) {
                            ++key_count;
                        } else {
                            last_key = escaped_key;
                            key_count = 1;
                        }
                        if(!this->process_key_press(escaped_key, key_count, 
                            return_code, escape_sequence_parameters))
                        {
                            return return_code;
                        }
                        escape_sequence.clear();
                        escape_sequence_parameters.clear();
                        break;
                    }
                } else if(enable_escape_sequences and key == '\033') {
                    last_key = -1;
                    key_count = 0;
                    escape_sequence.push_back(key);
                } else if(key == '\014') {
                    last_key = -1;
                    key_count = 0;
                    if(enable_escape_sequences) {
                        this->send_request_screen_size();
                        sent_request_window_size = true;
                    } else {
                        this->redraw();
                    }
                } else if(enable_reboot and key == '\002') {
                    last_key = -1;
                    key_count = 0;
                    RebootMenu reboot(this);
                    reboot.event_loop(/* enable_esc= */ true, /* enable_reboot= */ false);
                    this->redraw();
                } else {
                    if(sent_request_window_size) {
                        this->redraw();
                        sent_request_window_size = false;
                    }
                    if(key == last_key) {
                        ++key_count;
                    } else {
                        last_key = key;
                        key_count = 1;
                    }
                    if(!this->process_key_press(key, key_count, return_code, 
                            escape_sequence_parameters)) {
                        return return_code;
                    }                        
                }
                last_key_time = key_time;
            } else { /* timer occurred */
                // nothing happens here anymore
            }
        } else {
            if(was_connected) {
                if(!this->controller_disconnected(return_code)) {
                    return return_code;
                }
                was_connected = false;
                escape_sequence.clear();
                escape_sequence_parameters.clear();
                sent_request_window_size = false;
                last_key = -1;
                key_count = 0;
            }
            sleep_us(1000);
        }

        if(timer_delay <= 0) {
            if(!this->process_timer(was_connected, return_code)) {
                return return_code;
            }
            next_timer = delayed_by_us(next_timer, timer_interval_us_);
        }
        timer_delay = 
            std::max(absolute_time_diff_us(get_absolute_time(), next_timer), 0LL);
    }
    return return_code;
}

int Menu::decode_partial_escape_sequence(int key, std::string& escape_sequence,
    std::vector<std::string>& parameters)
{
    int escaped_key = -1;
    int escape_sequence_size = escape_sequence.size();
    if(escape_sequence_size == 1) {
        switch(key) {
        case '\033':
            escaped_key = '\033'; 
            break;
        case '[':
        case 'O':
            escape_sequence.push_back(key);
            escaped_key = INCOMPLETE_ESCAPE_SEQUENCE; 
            break;
        default: 
            escape_sequence.push_back(key);
            escaped_key = FAILED_ESCAPE_SEQUENCE; 
            break;
        }
    } else if(escape_sequence_size==2 and escape_sequence[1] == 'O') {
        switch(key) {
        // See https://www.gnu.org/software/screen/manual/html_node/Input-Translation.html#Input-Translation
        case 'A': escaped_key = KEY_UP; break;
        case 'B': escaped_key = KEY_DOWN; break;
        case 'C': escaped_key = KEY_RIGHT; break;
        case 'D': escaped_key = KEY_LEFT; break;
        case 'H': escaped_key = KEY_HOME; break;
        case 'F': escaped_key = KEY_END; break;
        case 'P': escaped_key = KEY_F1; break;
        case 'Q': escaped_key = KEY_F2; break;
        case 'R': escaped_key = KEY_F3; break;
        case 'S': escaped_key = KEY_F4; break;
        case 'p': escaped_key = '0'; break;
        case 'q': escaped_key = '1'; break;
        case 'r': escaped_key = '2'; break;
        case 's': escaped_key = '3'; break;
        case 't': escaped_key = '4'; break;
        case 'u': escaped_key = '5'; break;
        case 'v': escaped_key = '6'; break;
        case 'w': escaped_key = '7'; break;
        case 'x': escaped_key = '8'; break;
        case 'y': escaped_key = '9'; break;
        case 'k': escaped_key = '+'; break;
        case 'm': escaped_key = '-'; break;
        case 'j': escaped_key = '*'; break;
        case 'o': escaped_key = '/'; break;
        case 'X': escaped_key = '='; break;
        case 'n': escaped_key = '.'; break;
        case 'l': escaped_key = ','; break;
        case 'M': escaped_key = '\r'; break;
        default: 
            escape_sequence.push_back(key);
            escaped_key = UNSUPPORTED_ESCAPE_SEQUENCE; 
            break;
        }
    } else if(escape_sequence[1] == '[') {
        switch(key)
        {
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            if(parameters.empty())parameters.push_back({});
            parameters.back().push_back(key);
            escape_sequence.push_back(key);
            escaped_key = INCOMPLETE_ESCAPE_SEQUENCE;
            break;
        case ';':
            parameters.push_back({});
            escape_sequence.push_back(key);
            escaped_key = INCOMPLETE_ESCAPE_SEQUENCE;
            break;
        case ':': case '<': case '=': case '>': case '?': 
        case ' ': case '!': case '"': case '#': case '$': case '%':
        case '&': case '\'': case '(': case ')': case '*': case '+':
        case ',': case '-': case '.': case '/':
            escape_sequence.push_back(key);
            escaped_key = INCOMPLETE_ESCAPE_SEQUENCE;
            break;
        case 'A': escaped_key = KEY_UP; break;
        case 'B': escaped_key = KEY_DOWN; break;
        case 'C': escaped_key = KEY_RIGHT; break;
        case 'D': escaped_key = KEY_LEFT; break;
        case 'F': escaped_key = KEY_END; break;
        case 'H': escaped_key = KEY_HOME; break;
        case 'R': escaped_key = CURSOR_POSITION_REPORT; break;
        case '~':
            if(escape_sequence_size == 3) {
                switch(escape_sequence[2]) 
                {
                case '1': escaped_key = KEY_HOME; break;
                case '2': escaped_key = KEY_INSERT; break;
                case '3': escaped_key = KEY_DELETE; break;
                case '4': escaped_key = KEY_END; break;
                case '5': escaped_key = KEY_PAGE_UP; break;
                case '6': escaped_key = KEY_PAGE_DOWN; break;
                case '7': escaped_key = KEY_HOME; break;
                case '8': escaped_key = KEY_END; break;
                default: 
                    escape_sequence.push_back(key);
                    escaped_key = UNSUPPORTED_ESCAPE_SEQUENCE; break;
                    break;
                }
            } else if(escape_sequence_size==4 and escape_sequence[2]=='1') {
                switch(escape_sequence[3]) 
                {
                case '0': escaped_key = KEY_F0; break;
                case '1': escaped_key = KEY_F1; break;
                case '2': escaped_key = KEY_F2; break;
                case '3': escaped_key = KEY_F3; break;
                case '4': escaped_key = KEY_F4; break;
                case '5': escaped_key = KEY_F5; break;
                case '7': escaped_key = KEY_F6; break;
                case '8': escaped_key = KEY_F7; break;
                case '9': escaped_key = KEY_F8; break;
                default: 
                    escape_sequence.push_back(key);
                    escaped_key = UNSUPPORTED_ESCAPE_SEQUENCE; break;
                    break;
                }
            } else if(escape_sequence_size==4 and escape_sequence[2]=='2') {
                switch(escape_sequence[3]) 
                {
                case '0': escaped_key = KEY_F9; break;
                case '1': escaped_key = KEY_F10; break;
                case '3': escaped_key = KEY_F11; break;
                case '4': escaped_key = KEY_F12; break;
                default: 
                    escape_sequence.push_back(key);
                    escaped_key = UNSUPPORTED_ESCAPE_SEQUENCE; break;
                    break;
                }
            } else {
                escape_sequence.push_back(key);
                escaped_key = UNSUPPORTED_ESCAPE_SEQUENCE; break;
            }
            break;
        default:
            escape_sequence.push_back(key);
            if(key >= 0x40 and key <= 0x7F) {
                escaped_key = UNSUPPORTED_ESCAPE_SEQUENCE;
            } else {
                escaped_key = FAILED_ESCAPE_SEQUENCE;
            }
            break;
        }
    } else {
        escape_sequence.push_back(key);
        escaped_key = FAILED_ESCAPE_SEQUENCE;
    }
    return escaped_key;
}

FramedMenu::FramedMenu(const std::string& title, int frame_h, int frame_w, int frame_pos):
    title_(title), req_h_(frame_h), req_w_(frame_w), req_pos_(frame_pos)
{
    // nothing to see here
}

FramedMenu::~FramedMenu()
{
    // nothing to see here
}

void FramedMenu::redraw()
{
    setup_frame();
    hide_cursor();
    if(cls_on_redraw_)cls();
    draw_box(frame_h_, frame_w_, frame_r_, frame_c_);
    if(!title_.empty())draw_title(title_, frame_h_, frame_w_, frame_r_, frame_c_, {});
    if(heartbeat_) {
        draw_heart(true, frame_h_, frame_w_, frame_r_, frame_c_);
    }
}

void FramedMenu::set_heartbeat(bool on)
{
    if(heartbeat_ != on) {
        heartbeat_ = on;
        draw_heart(heartbeat_, frame_h_, frame_w_, frame_r_, frame_c_);
    }
}

void FramedMenu::setup_frame()
{
    frame_h_ = (req_h_>0) ? std::min(screen_h_, req_h_) : screen_h_;
    frame_w_ = (req_w_>0) ? std::min(screen_w_, req_w_) : screen_w_;
    switch(req_pos_) {
        case 0: case 3: case 7: default:
            frame_r_ = (screen_h_-frame_h_)/2; break;
        case 1: case 2: case 8: 
            frame_r_ = 0; break;
        case 4: case 5: case 6: 
            frame_r_ = screen_h_-frame_h_; break;
    }
    switch(req_pos_) {
        case 0: case 1: case 5: default:
            frame_c_ = (screen_w_-frame_w_)/2; break;
        case 2: case 3: case 4: 
            frame_c_ = screen_w_-frame_w_; break;
        case 6: case 7: case 8: 
            frame_c_ = 0; break;
    }
}

SimpleItemValueMenu::MenuItem::
MenuItem(const std::string& item_, int max_value_size_, const std::string& value_):
    item(item_), max_value_size(max_value_size_), value(value_)
{ 
    // nothing to see here
}

SimpleItemValueMenu::SimpleItemValueMenu(const std::vector<MenuItem>& menu_items, 
        const std::string& title, int frame_h, int frame_w, int frame_pos):
    FramedMenu(title, frame_h, frame_w, frame_pos), menu_items_(menu_items)
{
    // nothing to see here
}

SimpleItemValueMenu::~SimpleItemValueMenu()
{
    // nothing to see here
}

void SimpleItemValueMenu::setup_menu()
{
    item_r_ = frame_r_ + (title_.empty() ? 2 : 4);
    for(const auto& i : menu_items_) {
        item_w_ = std::max(item_w_, int(i.item.size()));
        val_w_ = std::max(val_w_, int(i.max_value_size));        
    }
    item_count_ = menu_items_.size();
    if(2*item_count_+item_r_+1 < frame_h_) {
        item_h_ = 2*item_count_-1;
        item_dr_ = 2;
    } else {
        item_h_ = std::min(item_count_, frame_h_-item_r_-2);
        item_dr_ = 1;
        item_count_ = item_h_;
    }
    item_w_ = std::min(item_w_, frame_w_ - val_w_ - 6);
    item_r_ += (frame_h_ - item_h_ - item_r_ - 2)/2;
    item_c_ = frame_c_ + std::min(5, frame_w_-(item_w_+val_w_+6)/2);
    val_c_ = frame_c_ + frame_w_ - val_w_ - (item_c_ - frame_c_);
}


void SimpleItemValueMenu::redraw()
{
    FramedMenu::redraw();
    setup_menu();
    for(int iitem=0;iitem<item_count_;++iitem) {
        draw_item(iitem);
    }
}

void SimpleItemValueMenu::draw_item(unsigned iitem)
{
    curpos(item_r_+iitem*item_dr_+1, item_c_+1);
    if(menu_items_[iitem].max_value_size > 0) {
        puts_raw_nonl(menu_items_[iitem].item, item_w_);
        putchar_raw(' ');
        for(int ic = item_c_+menu_items_[iitem].item.size()+2; ic<val_c_; ic ++)
            putchar_raw('.');
        putchar_raw(' ');
        draw_item_value(iitem);
    } else {
        puts_raw_nonl(menu_items_[iitem].item, item_w_+val_w_+2);
    }
}

void SimpleItemValueMenu::draw_item_value(unsigned iitem)
{
    if(iitem<menu_items_.size()) {
        curpos(item_r_+iitem*item_dr_+1, val_c_+1);
        if(!menu_items_[iitem].value_style.empty()) {
            puts_formatted(menu_items_[iitem].value, menu_items_[iitem].value_style, 
                menu_items_[iitem].max_value_size, true);
        } else {
            puts_raw_nonl(menu_items_[iitem].value, menu_items_[iitem].max_value_size, true);
        }
    }
}
