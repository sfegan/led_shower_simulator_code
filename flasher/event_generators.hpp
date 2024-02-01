#pragma once

#include <string>

#include "menu.hpp"
#include "event_dispatcher.hpp"

template<typename T> void lock_and_set(T& variable, const T& value) {
    EventDispatcher::instance().lock();
    variable = value;
    EventDispatcher::instance().unlock();
}

class EventGenerator {
public:
    virtual ~EventGenerator();
    virtual bool isEnabled() = 0;
    virtual void generateNextEvent() = 0;
    virtual uint64_t nextEventTime() = 0;
    virtual uint32_t* nextEventPattern() = 0;
};

class SingleLEDEventGenerator: public SimpleItemValueMenu {
public:
    SingleLEDEventGenerator();
    virtual ~SingleLEDEventGenerator();
    bool process_key_press(int key, int key_count, int& return_code, 
        const std::vector<std::string>& escape_sequence_parameters) final;
    bool process_timeout(int& return_code) final;

private:
    static std::vector<MenuItem> make_menu_items() {
        std::vector<MenuItem> menu_items;
        menu_items.emplace_back("F       : Set frequency mode (Poisson/Periodic)", 8, "Periodic");
        menu_items.emplace_back("+/-     : Increase/decrease frequency", 10, "100.0 Hz");
        menu_items.emplace_back("A       : Set LED amplitude mode (Random/Fixed)", 6, "Fixed");
        menu_items.emplace_back("</>     : Increase/decrease fixed LED amplitude", 3, "0");
        menu_items.emplace_back("P       : Set LED position mode (Random/Fixed)", 6, "Fixed");
        menu_items.emplace_back("Cursors : Change LED column & row", 3, "A1");
        menu_items.emplace_back("S       : Start (press and hold) or stop flasher", 4, "off");
        return menu_items;
    }

    void set_freq_mode_value(bool draw = true) { 
        if(freq_mode_ == 0) { menu_items_[0].value = "Periodic"; }
        else { menu_items_[0].value = "Poisson"; }
        if(draw)draw_item_value(0);
    }
    void set_freq_value(bool draw = true) { 
        char buffer[20];
        sprintf(buffer,"%.1f Hz",freq_);
        menu_items_[1].value = buffer; 
        if(draw)draw_item_value(1); 
    }

    void set_amp_mode_value(bool draw = true) { 
        if(amp_mode_ == 0) { menu_items_[2].value = "Fixed"; }
        else { menu_items_[2].value = "Random"; }
        if(draw)draw_item_value(2);
        set_amp_value(draw);
    }
    void set_amp_value(bool draw = true) { 
        if(amp_mode_ == 0) { menu_items_[3].value = std::to_string(amp_); }
        else { menu_items_[3].value = "N/A"; }
        if(draw)draw_item_value(3); 
    }

    void set_rc_mode_value(bool draw = true) { 
        if(rc_mode_ == 0) { menu_items_[4].value = "Fixed"; }
        else { menu_items_[4].value = "Random"; }
        if(draw)draw_item_value(4);
        set_rc_value(draw);
    }
    void set_rc_value(bool draw = true) { 
        if(rc_mode_ == 0) {
            menu_items_[5].value = std::string(1, char('A' + ar_)) 
                + std::to_string(ac_); }
        else { menu_items_[5].value = "N/A"; }
        if(draw)draw_item_value(5);
    }

    void set_enabled_value(bool draw = true) { 
        menu_items_[6].value = enabled_ ? ">ON<" : "off"; 
        menu_items_[6].value_style = enabled_ ? "\033[7m" : "";
        if(draw)draw_item_value(6); 
    }

    int freq_mode_ = 0;
    double freq_ = 100; // Hz
    int amp_mode_ = 0;
    int amp_ = 0;
    int rc_mode_ = 0;
    int ac_ = 0;
    int ar_ = 0;
    bool enabled_ = false;
};