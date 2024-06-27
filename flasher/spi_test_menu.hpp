#pragma once

#include <vector>

#include <pico/stdlib.h>

#include "flasher.hpp"
#include "menu.hpp"

class SPItestMenu: public SimpleItemValueMenu {
public:
    SPItestMenu();
    virtual ~SPItestMenu() { }
    bool controller_connected(int& return_code) final;
    bool controller_disconnected(int& return_code) final;
    bool process_key_press(int key, int key_count, int& return_code,
        const std::vector<std::string>& escape_sequence_parameters, absolute_time_t& next_timer) final;
    bool process_timer(bool controller_is_connected, int& return_code, absolute_time_t& next_timer) final;

private:
    enum MenuItemPositions {
        MIP_ROWCOL,
        MIP_DELAY,
        MIP_EXIT,
        MIP_NUM_ITEMS // MUST BE LAST ITEM IN LIST
    };

    std::vector<MenuItem> make_menu_items();

    void sync_values();
    void set_rc_value(bool draw = true);
    void set_delay_value(bool draw = true);

    int vdac_ = 0;
    int ac_ = 0;
    int ar_ = 0;
    int delay_ = 0;
    unsigned heartbeat_timer_count_ = 0;
};