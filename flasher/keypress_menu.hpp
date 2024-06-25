#pragma once

#include "menu.hpp"

class KeypressMenu: public Menu {
public:
    virtual ~KeypressMenu() { }
    void redraw() override;
    bool controller_connected(int& return_code) final;
    bool controller_disconnected(int& return_code) final;
    bool process_key_press(int key, int key_count, int& return_code,
        const std::vector<std::string>& escape_sequence_parameters,
        absolute_time_t& next_timer) final;
    bool process_timer(bool controller_is_connected, int& return_code,
        absolute_time_t& next_timer) final;
};
