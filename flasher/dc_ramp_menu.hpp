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
        MIP_PHASE,
        MIP_TIME,
        MIP_VDAC,
        MIP_EMPTY_LINE,
        MIP_ROWCOL,
        MIP_SCALE_DAC,
        MIP_TRIM_DAC,
        MIP_RAMP_UP,
        MIP_RAMP_HOLD,
        MIP_RAMP_DOWN,
        MIP_ENABLE_RAMP,
        MIP_EXIT,
        MIP_NUM_ITEMS // MUST BE LAST ITEM IN LIST
    };

    std::vector<MenuItem> make_menu_items();

    void sync_values();
    void set_rc_value(bool draw = true);
    void set_scale_value(bool draw = true);
    void set_offset_value(bool draw = true);
    void set_ramp_up_time_value(bool draw = true);
    void set_ramp_hold_time_value(bool draw = true);
    void set_ramp_down_time_value(bool draw = true);
    void set_enable_ramp_value(bool draw = true);
    void set_phase_value(bool draw = true);
    void set_time_value(bool draw = true);
    void set_vdac_value(bool draw = true);
    void delay();
    void configure_ramp();
    void unconfigure_ramp();

    int scale_ = 0;
    int offset_ = 0;
    int vdac_ = 0;
    int ac_ = 0;
    int ar_ = 0;
    int phase_ = 0;
    float ramp_up_time_ = 3;
    float ramp_hold_time_ = 5;
    float ramp_down_time_ = 3;
    float time_ = 0;
    bool enable_ramp_ = 0;
    unsigned heartbeat_timer_count_ = 0;
};