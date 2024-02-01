#include <cmath>
#include <pico/double.h>

#include "event_generators.hpp"

EventGenerator::~EventGenerator()
{
    // nothing to see here
}

SingleLEDEventGenerator::SingleLEDEventGenerator(): 
    SimpleItemValueMenu(make_menu_items(), "Single LED event generator") 
{
    // nothing to see here
}

SingleLEDEventGenerator::~SingleLEDEventGenerator()
{
    // nothing to see here
}

bool SingleLEDEventGenerator::process_key_press(int key, int key_count, int& return_code, 
    const std::vector<std::string>& escape_sequence_parameters)
{
    switch(key) {
    case 'F':
        freq_mode_ = (freq_mode_ == 0) ? 1 : 0;
        set_freq_mode_value();
        break;
    case '+':
        if(freq_<30000.0) {
            double df = 0.1;
            if(freq_ >= 3000 || (freq_ >= 300 && key_count>10)) { df = 100; }
            else if(freq_ >= 300 || (freq_ >= 30 && key_count>10)) { df = 10.0; }
            else if(freq_ >= 30 || key_count>10) { df = 1.0; }
            freq_ = std::min((std::floor(freq_/df + 0.5) + 1.0) * df, 30000.0);
            set_freq_value();
        }
        break;
    case '-':
    case '_':
        if(freq_>0.0) {
            double df = 0.1;
            if(freq_ > 3000 || (freq_ > 300 && key_count>10)) { df = 100; }
            else if(freq_ > 300 || (freq_ > 30 && key_count>10)) { df = 10.0; }
            else if(freq_ > 30 || key_count>10) { df = 1.0; }
            freq_ = std::max((std::floor(freq_/df + 0.5) - 1.0) * df, 0.0);
            set_freq_value();
        }
        break;
    case 'A':
        amp_mode_ = (amp_mode_ == 0) ? 1 : 0;
        set_amp_mode_value();
        break;
    case '>':
        if(amp_mode_ == 0 and amp_<255) {
            amp_ = std::min(amp_ + (key_count >= 15 ? 5 : 1), 255);
            set_amp_value();
        }
        break;
    case '<':
        if(amp_mode_ == 0 and amp_>0) {
            amp_ = std::max(amp_ - (key_count >= 15 ? 5 : 1), 0);
            set_amp_value();
        }
        break;
    case 'P':
        rc_mode_ = (rc_mode_ == 0) ? 1 : 0;
        set_rc_mode_value();
        break;
    case KEY_UP:
        if(rc_mode_ == 0 and ar_>0) {
            ar_ = std::max(ar_-1, 0);
            set_rc_value();
        }
        break;
    case KEY_DOWN:
        if(rc_mode_ == 0 and ar_<15) {
            ar_ = std::min(ar_+1, 15);
            set_rc_value();
        }
        break;
    case KEY_LEFT:
        if(rc_mode_ == 0 and ac_>0) {
            ac_ = std::max(ac_-1, 0);
            set_rc_value();
        }
        break;
    case KEY_RIGHT:
        if(rc_mode_ == 0 and ac_<15) {
            ac_ = std::min(ac_+1, 15);
            set_rc_value();
        }
        break;
    case KEY_PAGE_UP:
        if(rc_mode_ == 0 and ar_>0) {
            ar_ = 0;
            set_rc_value();
        }
        break;
    case KEY_PAGE_DOWN:
        if(rc_mode_ == 0 and ar_<15) {
            ar_ = 15;
            set_rc_value();
        }
        break;
    case KEY_HOME:
        if(rc_mode_ == 0 and ac_>0) {
            ac_ = 0;
            set_rc_value();
        }
        break;
    case KEY_END:
        if(rc_mode_ == 0 and ac_<15) {
            ac_ = 15;
            set_rc_value();
        }
        break;
    case 'S':
        if(enabled_ and key_count == 1) {
            enabled_ = false;
            set_enabled_value();
        } else if(key_count >= 10) {
            enabled_ = true;
            set_enabled_value();
        }
        break;
    }
    return true;
}

bool SingleLEDEventGenerator::process_timeout(int& return_code)
{
    return true;
}
