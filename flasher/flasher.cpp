#include <string>
#include <vector>

#include <cstring>
#include <cstdio>
#include <cctype>

#include <pico/stdlib.h>

class Menu {
public:
    virtual ~Menu() { }
    virtual void redraw() = 0;
    virtual bool process_key_press(int key, int key_count, int& return_code) = 0;
    virtual bool process_timeout(int& return_code) = 0;

    static int puts_raw_nonl(const char* s) {
        for (size_t i = 0; s[i]; ++i) {
            if (putchar_raw(s[i]) == EOF) return EOF;
        }
        return 0;
    }
    static int puts_raw_nonl(const char* s, size_t maxchars, bool fill = false) {
        for (size_t i = 0; s[i] && maxchars; ++i, --maxchars) {
            if (putchar_raw(s[i]) == EOF) return EOF;
        }
        if(fill && maxchars) {
            while(maxchars--) {
                if (putchar_raw(' ') == EOF) return EOF;
            }
        }
        return 0;
    }
    static int puts_raw_nonl(const std::string& s) {
        for (size_t i=0; i<s.size(); ++i) {
            if (putchar_raw(s[i]) == EOF) return EOF;
        }
        return 0;
    }
    static int puts_raw_nonl(const std::string& s, size_t maxchars, bool fill = false) {
        size_t schars = std::min(maxchars, s.size());
        for (size_t i=0; i<schars; ++i, --maxchars) {
            if (putchar_raw(s[i]) == EOF) return EOF;
        }
        if(fill && maxchars) {
            while(maxchars--) {
                if (putchar_raw(' ') == EOF) return EOF;
            }
        }
        return 0;
    }

    static int screen_width() { return 80; }
    static int screen_height() { return 25; }
    static void cls() { 
        puts_raw_nonl("\033[2J"); 
    }
    static void show_cursor() { 
        puts_raw_nonl("\033[?25h\0337p"); 
    }
    static void hide_cursor() { 
        puts_raw_nonl("\033[?25l\0336p"); 
    }
    static void curpos(int r, int c) { 
        char buffer[80]; 
        sprintf(buffer,"\033[%d;%dH",r,c); 
        puts_raw_nonl(buffer);
    }
    static void set_screen_size(int h=screen_height(), int w=screen_width()) {
        char buffer[80]; 
        sprintf(buffer,"\033[8;%d;%dt",h,w); 
        puts_raw_nonl(buffer);
    }

    static void draw_frame(int fh=screen_height(), int fw=screen_width(), 
            int fr=-1, int fc=-1) {
        char buffer[80];
        if(fr<0) { fr = std::max((screen_height()-fh)/2, 0); }
        if(fc<0) { fc = std::max((screen_width()-fw)/2, 0); }
        sprintf(buffer,"\033[%d;%dH",fr+1,fc+1);
        puts_raw_nonl(buffer);
        putchar_raw('+');
        for(int ic=2;ic<fw;++ic)putchar_raw('-');
        putchar_raw('+');
        for(int ir=2;ir<fh;++ir) {
            sprintf(buffer,"\033[%d;%dH|\033[%d;%dH|",fr+ir,fc+1,fr+ir,fc+fw);
            puts_raw_nonl(buffer);
        }
        sprintf(buffer,"\033[%d;%dH",fr+fh,fc+1);
        puts_raw_nonl(buffer);
        putchar_raw('+');
        for(int ic=2;ic<screen_width();++ic)putchar_raw('-');
        putchar_raw('+');
    }

    static bool draw_title(const std::string& title, const std::string& title_style = {},
            int fh=screen_height(), int fw=screen_width(), 
            int fr=-1, int fc=-1) {
        char buffer[80];
        if(fr<0 ){ fr = std::max((screen_height()-fh)/2, 0); }
        if(fc<0) { fc = std::max((screen_width()-fw)/2, 0); }
        if(fh<5 || fw<5)return false;
        int tw = std::min(int(title.size()), fw-4);
        int tc = fc + (fw-tw)/2;
        sprintf(buffer,"\033[%d;%dH",fr+3,tc+1);
        puts_raw_nonl(buffer);
        if(!title_style.empty())puts_raw_nonl(title_style);
        puts_raw_nonl(title, tw);
        if(!title_style.empty())puts_raw_nonl("\033[0m");
        return tw == int(title.size());
    }
};

class SimpleValueMenu: public Menu {
public:
    struct MenuItem {
        MenuItem(const std::string& item_, int max_value_size_, const std::string& value_={}):
            item(item_), max_value_size(max_value_size_), value(value_) { }
        std::string item;
        int max_value_size;
        std::string value;
    };

    SimpleValueMenu(const std::vector<MenuItem>& menu_items, const std::string& title,
        int frame_h=screen_height(), int frame_w=screen_width(), int frame_r=-1, int frame_c=-1);
    virtual ~SimpleValueMenu() { }
    void redraw() override;
    virtual bool process_key_press(int key, int key_count, int& return_code) = 0;
    virtual bool process_timeout(int& return_code) = 0;

protected:
    int frame_h_;
    int frame_w_;
    int frame_r_;
    int frame_c_;

    std::string title_;
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

SimpleValueMenu::SimpleValueMenu(const std::vector<MenuItem>& menu_items, 
        const std::string& title, int frame_h, int frame_w, int frame_r, int frame_c):
    Menu(), frame_h_(frame_h), frame_w_(frame_w), frame_r_(frame_r), frame_c_(frame_c),
    title_(title), menu_items_(menu_items)
{
    if(frame_r_<0){ frame_r_ = std::max((screen_height()-frame_h_)/2, 0); }
    if(frame_c_<0){ frame_c_ = std::max((screen_width()-frame_w_)/2, 0); }
    item_r_ = frame_r_ + (title.empty() ? 2 : 4);
    for(const auto& i : menu_items) {
        item_w_ = std::max(item_w_, int(i.item.size()));
        val_w_ = std::max(val_w_, int(i.max_value_size));        
    }
    item_count_ = menu_items.size();
    if(2*item_count_+item_r_+1 < frame_h_) {
        item_h_ = 2*item_count_-1;
        item_dr_ = 2;
    } else {
        item_h_ = std::max(item_count_, frame_h-item_r_-2);
        item_dr_ = 2;
        item_count_ = item_h_;
    }
    item_w_ = std::min(item_w_, frame_w_ - val_w_ - 6);
    item_c_ = frame_c_ + 2;
    val_c_ = frame_c_ + frame_w_ - val_w_ - 2;
}

void SimpleValueMenu::redraw()
{
    set_screen_size();
    hide_cursor();
    cls();
    draw_frame(frame_h_, frame_w_, frame_r_, frame_c_);
    if(!title_.empty())draw_title(title_, {}, frame_h_, frame_w_, frame_r_, frame_c_);
    for(int i=0;i<item_count_;++i) {
        curpos(item_r_+i*item_dr_+1, item_c_+1);
        puts_raw_nonl(menu_items_[i].item, item_w_);
        if(menu_items_[i].max_value_size > 0) {
            curpos(item_r_+i*item_dr_+1, val_c_+1);
            puts_raw_nonl(menu_items_[i].value, menu_items_[i].max_value_size, true);
        }
    }
}

class EngineeringMenu: public SimpleValueMenu {
public:
    EngineeringMenu() : SimpleValueMenu(make_menu_items(), "Engineering menu") { }
    virtual ~EngineeringMenu() { }
    bool process_key_press(int key, int key_count, int& return_code) final;
    bool process_timeout(int& return_code) final;
private:
    static std::vector<MenuItem> make_menu_items() {
        std::vector<MenuItem> menu_items;
        menu_items.emplace_back("-/+ : Increase/decrease DAC voltage", 3, "0");
        menu_items.emplace_back("h/l : Decrease/increase column", 2, "1");
        menu_items.emplace_back("j/k : Increase/decrease row", 2, "1");
        return menu_items;
    }
    int vdac_ = 0;
    int ac_ = 0;
    int ar_ = 0;
    int trig_ = 0;
    bool dac_e_ = 0;
};

bool EngineeringMenu::process_key_press(int key, int key_count, int& return_code)
{
    char buffer[80];
    if(isprint(key)) {
        sprintf(buffer,"%c %d %d        ",char(key),key,key_count);
    } else {
        sprintf(buffer,"%c %d %d        ",' ',key,key_count);        
    }
    curpos(3,3);
    puts_raw_nonl(buffer);
    return true;
}

bool EngineeringMenu::process_timeout(int& return_code)
{
    return true;
}

int event_loop(Menu& menu)
{
    int return_code = 0;
    bool continue_looping = true;
    bool was_connected = false;
    int last_key = -1;
    int key_count = 0;
    while(continue_looping) {
        if(stdio_usb_connected()) {
            if(!was_connected) {
                menu.redraw();
            }
            was_connected = true;
            int key = getchar_timeout_us(100000);
            if(key >= 0) { 
                if(key == '\014') {
                    menu.redraw();
                    continue;
                }
                if(key == last_key) {
                    ++key_count;
                } else {
                    last_key = key;
                    key_count = 1;
                }
                continue_looping = menu.process_key_press(key, key_count, return_code);
            } else {
                last_key = -1;
                key_count = 0;
                continue_looping = menu.process_timeout(return_code);
            }
        } else {
            was_connected = false;
            sleep_us(1000);
        }
    }

    return continue_looping;
}


int main()
{
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    stdio_init_all();

    EngineeringMenu menu;
    event_loop(menu);
}
