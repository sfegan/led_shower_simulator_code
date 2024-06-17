#pragma once

#include <vector>

#include "menu.hpp"

class MainMenu: public SimpleItemValueMenu {
public:
    MainMenu();
    virtual ~MainMenu();
    bool controller_connected(int& return_code) final;
    bool controller_disconnected(int& return_code) final;
    bool process_key_press(int key, int key_count, int& return_code,
        const std::vector<std::string>& escape_sequence_parameters) final;
    bool process_timer(bool controller_is_connected, int& return_code) final;

private:
    enum MenuItemPositions {
        MIP_ENGINEERING,
        MIP_DC_RAMP,
        MIP_REBOOT,
        MIP_NUM_ITEMS // MUST BE LAST ITEM IN LIST
    };

    static std::vector<MenuItem> make_menu_items();
};
