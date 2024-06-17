#pragma once

#include <vector>

#include <pico/stdlib.h>

#include "flasher.hpp"
#include "menu.hpp"

class DCRampMenu: public SimpleItemValueMenu {
public:
    DCRampMenu();
    virtual ~DCRampMenu() { }
    bool controller_connected(int& return_code) final;
    bool controller_disconnected(int& return_code) final;
    bool process_key_press(int key, int key_count, int& return_code,
        const std::vector<std::string>& escape_sequence_parameters) final;
    bool process_timer(bool controller_is_connected, int& return_code) final;

private:
    enum MenuItemPositions {
        MIP_ROWCOL,
        MIP_SCALE_DAC,
        MIP_TRIM_DAC,
        MIP_EXIT,
        MIP_NUM_ITEMS // MUST BE LAST ITEM IN LIST
    };

    std::vector<MenuItem> make_menu_items();

    void sync_values();
    void set_rc_value(bool draw = true);
    void set_scale_value(bool draw = true);
    void set_offset_value(bool draw = true);

    int scale_ = 0;
    int offset_ = 0;
    int ac_ = 0;
    int ar_ = 0;
    unsigned heartbeat_timer_count_ = 0;
};