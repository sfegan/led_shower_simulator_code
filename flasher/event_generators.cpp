#include <cmath>
#include <pico/double.h>

#include "build_date.hpp"
#include "event_generators.hpp"
#include "event_dispatcher.hpp"

namespace {
    static BuildDate build_date(__DATE__,__TIME__);
}

template<typename T> void lock_and_set(T& variable, const T& value) {
    EventDispatcher::instance().lock();
    variable = value;
    EventDispatcher::instance().unlock();
}

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

bool SingleLEDEventGenerator::isEnabled()
{
    return enabled_;
}

void SingleLEDEventGenerator::generateNextEvent()
{
    // nothing to see here
}

uint32_t SingleLEDEventGenerator::nextEventDelay()
{
    if(freq_mode_ == 0) {
        return period_us_;
    } else {
        return -std::log(double(rand())/double(RAND_MAX))*period_us_;
    }
}

uint32_t SingleLEDEventGenerator::nextEventPattern(uint32_t* array)
{
    uint x = rand() & 0xFFFF;
    if(amp_mode_ == 0)x = (x&0xFF00) | (amp_&0x00FF);
    if(rc_mode_ == 0)x = (x&0x00FF) | ((ar_&0x000F)<<8) | ((ac_&0x000F)<<12);
    array[0] = x;
    return 1;
}

bool SingleLEDEventGenerator::process_key_press(int key, int key_count, int& return_code, 
    const std::vector<std::string>& escape_sequence_parameters, 
    absolute_time_t& next_timer)
{
    switch(key) {
    case 'F':
        freq_mode_ = (freq_mode_ == 0) ? 1 : 0;
        set_freq_mode_value();
        break;
    case '+':
        if(freq_<30000.0) {
            double df = 0.1;
            if(freq_ >= 3000 && key_count>10) { df = 1000; }
            else if(freq_ >= 3000 || (freq_ >= 300 && key_count>10)) { df = 100; }
            else if(freq_ >= 300 || (freq_ >= 30 && key_count>10)) { df = 10.0; }
            else if(freq_ >= 30 || key_count>10) { df = 1.0; }
            lock_and_set(freq_, std::min((std::floor(freq_/df + 0.5) + 1.0) * df, 30000.0));
            lock_and_set(period_us_, 1000000.0/freq_);
            set_freq_value();
        }
        break;
    case '-':
    case '_':
        if(freq_>0.0) {
            double df = 0.1;
            if(freq_ > 3000 && key_count>10) { df = 1000; }
            else if(freq_ > 3000 || (freq_ > 300 && key_count>10)) { df = 100; }
            else if(freq_ > 300 || (freq_ > 30 && key_count>10)) { df = 10.0; }
            else if(freq_ > 30 || key_count>10) { df = 1.0; }
            lock_and_set(freq_, std::max((std::floor(freq_/df + 0.5) - 1.0) * df, 0.0));
            lock_and_set(period_us_, 1000000.0/freq_);
            set_freq_value();
        }
        break;
    case '0': case '1': case '2': case '3': case '4': case '5':
        if(key_count >= 10) {
            double new_freq = 0.1;
            while(key > '0') { new_freq *= 10.0; --key; }
            if(freq_ != new_freq) {
                lock_and_set(freq_, new_freq);
                lock_and_set(period_us_, 1000000.0/freq_);
                set_freq_value();
            }
        }
        break;
    case 'A':
        lock_and_set(amp_mode_, (amp_mode_ == 0) ? 1 : 0);
        set_amp_mode_value();
        break;
    case '>':
        if(amp_mode_ == 0 and amp_<255) {
            lock_and_set(amp_, std::min(amp_ + (key_count >= 15 ? 5 : 1), 255));
            set_amp_value();
        }
        break;
    case '<':
        if(amp_mode_ == 0 and amp_>0) {
            lock_and_set(amp_, std::max(amp_ - (key_count >= 15 ? 5 : 1), 0));
            set_amp_value();
        }
        break;
    case 'P':
        lock_and_set(rc_mode_, (rc_mode_ == 0) ? 1 : 0);
        set_rc_mode_value();
        break;
    case KEY_UP:
        if(rc_mode_ == 0 and ar_>0) {
            lock_and_set(ar_, std::max(ar_-1, 0));
            set_rc_value();
        }
        break;
    case KEY_DOWN:
        if(rc_mode_ == 0 and ar_<15) {
            lock_and_set(ar_, std::min(ar_+1, 15));
            set_rc_value();
        }
        break;
    case KEY_LEFT:
        if(rc_mode_ == 0 and ac_>0) {
            lock_and_set(ac_, std::max(ac_-1, 0));
            set_rc_value();
        }
        break;
    case KEY_RIGHT:
        if(rc_mode_ == 0 and ac_<15) {
            lock_and_set(ac_, std::min(ac_+1, 15));
            set_rc_value();
        }
        break;
    case KEY_PAGE_UP:
        if(rc_mode_ == 0 and ar_>0) {
            lock_and_set(ar_, 0);
            set_rc_value();
        }
        break;
    case KEY_PAGE_DOWN:
        if(rc_mode_ == 0 and ar_<15) {
            lock_and_set(ar_, 15);
            set_rc_value();
        }
        break;
    case KEY_HOME:
        if(rc_mode_ == 0 and ac_>0) {
            lock_and_set(ac_, 0);
            set_rc_value();
        }
        break;
    case KEY_END:
        if(rc_mode_ == 0 and ac_<15) {
            lock_and_set(ac_, 15);
            set_rc_value();
        }
        break;
    case 'S':
        if(enabled_ and key_count == 1) {
            lock_and_set(enabled_, false);
            set_enabled_value();
        } else if(key_count >= 10) {
            lock_and_set(enabled_, true);
            set_enabled_value();
        }
        break;
    case 's':
        if(enabled_) {
            lock_and_set(enabled_, false);
            set_enabled_value();
        }
        break;
    }
    return true;
}

bool SingleLEDEventGenerator::process_timer(bool controller_is_connected, int& return_code,
    absolute_time_t& next_timer)
{
    return true;
}
