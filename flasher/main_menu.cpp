#include "menu.hpp"
#include "main_menu.hpp"
#include "keypress_menu.hpp"
#include "engineering_menu.hpp"

#define WRITEVAL(x) \
    { \
        char buffer[80]; \
        sprintf(buffer, "%-20s : %d\n\r", #x, x); \
        puts_raw_nonl(buffer); \
    }

std::vector<SimpleItemValueMenu::MenuItem> MainMenu::make_menu_items() {
    std::vector<SimpleItemValueMenu::MenuItem> menu_items(MIP_NUM_ITEMS);
    menu_items.at(MIP_ENGINEERING) = {"E/e     : Engineering menu", 0, ""};
    menu_items.at(MIP_REBOOT)      = {"Ctrl-b  : Reboot flasher (press and hold)", 0, ""};
    return menu_items;
}

MainMenu::MainMenu():
    SimpleItemValueMenu(make_menu_items(), "LLR 256-pixel flasher : Main menu") 
{
    timer_interval_us_ = 1000000; // 1Hz
}
    
MainMenu::~MainMenu()
{
    // nothing to see here
}

bool MainMenu::controller_connected(int& return_code)
{
    return true;
}

bool MainMenu::controller_disconnected(int& return_code)
{
    return true;
}

bool MainMenu::process_key_press(int key, int key_count, int& return_code,
    const std::vector<std::string>& escape_sequence_parameters)
{
    switch(key) {
    case 'E': 
    case 'e': 
        {
            EngineeringMenu menu;
            menu.event_loop();
            this->redraw();
        }
        break;
    case 11: /* ctrl-K : secret keypress menu */
        {
            KeypressMenu menu;
            menu.event_loop();
            this->redraw();
        }
        break;
    case 7: /* ctrl-g : secret display of menu parameters - to remove */
        cls();
        curpos(1,1);
        WRITEVAL(req_h_);
        WRITEVAL(req_w_);
        WRITEVAL(req_pos_);
        WRITEVAL(screen_h_);
        WRITEVAL(screen_w_);
        WRITEVAL(frame_h_);
        WRITEVAL(frame_w_);
        WRITEVAL(frame_r_);
        WRITEVAL(frame_c_);
        WRITEVAL(item_count_);
        WRITEVAL(item_h_);
        WRITEVAL(item_w_);
        WRITEVAL(val_w_);
        WRITEVAL(item_r_);
        WRITEVAL(item_c_);
        WRITEVAL(val_c_);
        WRITEVAL(item_dr_);
        puts("Press ctrl-L to redraw menu...");
        break;
    }
    return true;
}

bool MainMenu::process_timer(bool controller_is_connected, int& return_code)
{
    if(controller_is_connected) {
        set_heartbeat(!heartbeat_);
    }
    return true;
}
