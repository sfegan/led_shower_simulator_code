#pragma once

#include <string>
#include <vector>

#define ANSI_INVERT "\033[7m"

class Menu {
public:
    virtual ~Menu();
    virtual void redraw() = 0;
    virtual bool controller_connected(int& return_code) = 0;
    virtual bool controller_disconnected(int& return_code) = 0;
    virtual bool process_key_press(int key, int key_count, int& return_code,
        const std::vector<std::string>& escape_sequence_parameters) = 0;
    virtual bool process_timer(bool controller_is_connected, int& return_code) = 0;

    int event_loop(bool enable_escape_sequences = true, bool enable_reboot = true);

    uint64_t timer_interval_us() const { return timer_interval_us_; }
    int screen_width() const { return screen_w_; }
    int screen_height() const { return screen_h_; }
    void set_screen_size(int h, int w) { screen_h_ = h; screen_w_ = w; }

    static int puts_raw_nonl(const char* s);
    static int puts_raw_nonl(const char* s, size_t maxchars, bool fill = false);
    static int puts_raw_nonl(const std::string& s);
    static int puts_raw_nonl(const std::string& s, size_t maxchars, bool fill = false);

    static int puts_formatted(const std::string& s, const std::string& format, 
        size_t maxchars, bool fill = false);

    static int puts_center_filled(const std::string& s, size_t maxchars, char fill_char = ' ');

    static int default_screen_width() { return 80; }
    static int default_screen_height() { return 24; }
    static uint64_t default_timer_interval_us() { return 10000; } /* 100Hz */

    static void cls();
    static void show_cursor();
    static void hide_cursor();
    static void curpos(int r, int c);
    static void save_cursor();
    static void restore_cursor();
    static void highlight();
    static void reset_colors();
    static void send_request_screen_size();

    static void draw_box(int fh, int fw, int fr, int fc);
    static bool draw_title(const std::string& title,
            int fh, int fw, int fr, int fc, const std::string& title_style = {});
    static bool draw_heart(bool on, int fh, int fw, int fr, int fc);

    static const int FAILED_ESCAPE_SEQUENCE      = 997;
    static const int INCOMPLETE_ESCAPE_SEQUENCE  = 998;
    static const int UNSUPPORTED_ESCAPE_SEQUENCE = 999;
    static const int KEY_UP                      = 1000;
    static const int KEY_DOWN                    = 1001;
    static const int KEY_RIGHT                   = 1002;
    static const int KEY_LEFT                    = 1003;
    static const int KEY_HOME                    = 1004;
    static const int KEY_END                     = 1005;
    static const int KEY_PAGE_UP                 = 1006;
    static const int KEY_PAGE_DOWN               = 1007;
    static const int KEY_INSERT                  = 1008;
    static const int KEY_DELETE                  = 1008;
    static const int KEY_F0                      = 1020;
    static const int KEY_F1                      = 1021;
    static const int KEY_F2                      = 1022;
    static const int KEY_F3                      = 1023;
    static const int KEY_F4                      = 1024;
    static const int KEY_F5                      = 1025;
    static const int KEY_F6                      = 1026;
    static const int KEY_F7                      = 1027;
    static const int KEY_F8                      = 1028;
    static const int KEY_F9                      = 1029;
    static const int KEY_F10                     = 1030;
    static const int KEY_F11                     = 1031;
    static const int KEY_F12                     = 1032;
    static const int CURSOR_POSITION_REPORT      = 1100;

protected:
    uint64_t timer_interval_us_   = default_timer_interval_us();
    int screen_w_                 = default_screen_width();
    int screen_h_                 = default_screen_height();

private:
    static int decode_partial_escape_sequence(int key, std::string& escape_sequence, 
        std::vector<std::string>& escape_sequence_parameters);
};

class FramedMenu: public Menu
{
public:
    FramedMenu(const std::string& title={}, int frame_h=0, int frame_w=0, int frame_pos=0);
    virtual ~FramedMenu();

    void redraw() override;

    void set_heartbeat(bool on);

protected:
    void setup_frame();

    std::string title_;
    
    int req_h_;
    int req_w_;
    int req_pos_;

    int frame_h_ = default_screen_height();
    int frame_w_ = default_screen_width();
    int frame_r_ = 0;
    int frame_c_ = 0;

    bool cls_on_redraw_ = false;
    bool heartbeat_ = false;
};

class SimpleItemValueMenu: public FramedMenu {
public:
    struct MenuItem {
        MenuItem(): item(), max_value_size(), value(), value_style() {}
        MenuItem(const std::string& item_, int max_value_size_, const std::string& value_={});
        std::string item;
        int max_value_size;
        std::string value;
        std::string value_style = {};
    };

    SimpleItemValueMenu(const std::vector<MenuItem>& menu_items, 
        const std::string& title={}, int frame_h=0, int frame_w=0, int frame_pos=0);
    virtual ~SimpleItemValueMenu();
    
    void redraw() override;
    
protected:
    void draw_item(unsigned iitem);
    void draw_item_value(unsigned iitem);

    void setup_menu();

    std::vector<MenuItem> menu_items_;
    
    int item_count_ = 0;
    int item_h_ = 0;
    int item_w_ = 0;
    int val_w_ = 0;
    int item_r_ = 0;
    int item_c_ = 0;
    int val_c_ = 0;
    int item_dr_ = 1;
};
