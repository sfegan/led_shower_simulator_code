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

    int event_loop();

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
    static int screen_height() { return 24; }
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

int Menu::event_loop()
{
    int return_code = 0;
    bool continue_looping = true;
    bool was_connected = false;
    int last_key = -1;
    int key_count = 0;
    while(continue_looping) {
        if(stdio_usb_connected()) {
            if(!was_connected) {
                this->redraw();
            }
            was_connected = true;
            int key = getchar_timeout_us(100000);
            if(key >= 0) { 
                if(key == '\014') {
                    this->redraw();
                    continue;
                }
                if(key == last_key) {
                    ++key_count;
                } else {
                    last_key = key;
                    key_count = 1;
                }
                continue_looping = this->process_key_press(key, key_count, return_code);
            } else {
                last_key = -1;
                key_count = 0;
                continue_looping = this->process_timeout(return_code);
            }
        } else {
            was_connected = false;
            sleep_us(1000);
        }
    }

    return return_code;
}

class KeypressMenu: public Menu {
public:
    virtual ~KeypressMenu() { }
    void redraw() override;
    bool process_key_press(int key, int key_count, int& return_code) override;
    bool process_timeout(int& return_code) override;
};

void KeypressMenu::redraw()
{
    cls();
    curpos(1,1);
    puts("Type some keys (terminate with Crtl-D)");
}

bool KeypressMenu::process_key_press(int key, int key_count, int& return_code)
{
    char buffer[80];
    sprintf(buffer, "%c %d \\%o %d",isprint(key)?key:' ',key,key,key_count);
    puts(buffer);
    return_code = 0;
    return key != '\004';
}

bool KeypressMenu::process_timeout(int& return_code)
{
    return_code = 0;
    return true;
}

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

    void draw_item(unsigned iitem);
    void draw_item_value(unsigned iitem);

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
        item_h_ = std::min(item_count_, frame_h-item_r_-2);
        item_dr_ = 1;
        item_count_ = item_h_;
    }
    item_w_ = std::min(item_w_, frame_w_ - val_w_ - 6);
    item_r_ += (frame_h_ - item_h_ - item_r_ - 2)/2;
    item_c_ = frame_c_ + std::min(5, frame_w_-(item_w_+val_w_+6)/2);
    val_c_ = frame_c_ + frame_w_ - val_w_ - (item_c_ - frame_c_);
}

#define WRITEVAL(x) \
    { \
        char buffer[80]; \
        sprintf(buffer, #x " : %d\n\r", x); \
        puts_raw_nonl(buffer); \
    }

void SimpleValueMenu::redraw()
{
    set_screen_size();
    hide_cursor();
    cls();
    draw_frame(frame_h_, frame_w_, frame_r_, frame_c_);
    if(!title_.empty())draw_title(title_, {}, frame_h_, frame_w_, frame_r_, frame_c_);
    for(int iitem=0;iitem<item_count_;++iitem) {
        draw_item(iitem);
    }
}

void SimpleValueMenu::draw_item(unsigned iitem)
{
    curpos(item_r_+iitem*item_dr_+1, item_c_+1);
    if(menu_items_[iitem].max_value_size > 0) {
        puts_raw_nonl(menu_items_[iitem].item, item_w_);
        putchar_raw(' ');
        for(int ic = item_c_+menu_items_[iitem].item.size()+2; ic<val_c_; ic ++)
            putchar_raw('.');
        putchar_raw(' ');
        draw_item_value(iitem);
    } else {
        puts_raw_nonl(menu_items_[iitem].item, item_w_+val_w_+2);
    }
}

void SimpleValueMenu::draw_item_value(unsigned iitem)
{
    if(iitem<menu_items_.size()) {
        curpos(item_r_+iitem*item_dr_+1, val_c_+1);
        puts_raw_nonl(menu_items_[iitem].value, menu_items_[iitem].max_value_size, true);
    }
}

class EngineeringMenu: public SimpleValueMenu {
public:
    EngineeringMenu();
    virtual ~EngineeringMenu() { }
    bool process_key_press(int key, int key_count, int& return_code) final;
    bool process_timeout(int& return_code) final;

private:
    static std::vector<MenuItem> make_menu_items() {
        std::vector<MenuItem> menu_items;
        menu_items.emplace_back("+/- : Increase/decrease DAC voltage", 3, "0");
        menu_items.emplace_back("Z   : Zero DAC voltage", 0, "");
        menu_items.emplace_back("h/l : Decrease/increase column", 2, "1");
        menu_items.emplace_back("j/k : Increase/decrease row", 2, "1");
        menu_items.emplace_back("D   : Toggle DAC enabled", 3, "off");
        menu_items.emplace_back("T   : Toggle trigger", 3, "off");
        menu_items.emplace_back("P   : Pulse trigger", 0, "");
        menu_items.emplace_back("L   : Toggle on-board LED", 3, "off");
        menu_items.emplace_back("K   : Display keypress", 0);
        //menu_items.emplace_back("Second test line", 0);
        //menu_items.emplace_back("Third test line", 0);
        return menu_items;
    }

    void sync_values();
    void set_vdac_value(bool draw = true) { 
        menu_items_[0].value = std::to_string(vdac_); if(draw)draw_item_value(0); }
    void set_ac_value(bool draw = true) { 
        menu_items_[2].value = std::to_string(ac_); if(draw)draw_item_value(2); }
    void set_ar_value(bool draw = true) { 
        menu_items_[3].value = std::to_string(ar_); if(draw)draw_item_value(4); }
    void set_dac_e_value(bool draw = true) { 
        menu_items_[4].value = dac_e_ ? "on" : "off"; if(draw)draw_item_value(4); }
    void set_trig_value(bool draw = true) { 
        menu_items_[5].value = trig_ ? "on" : "off"; if(draw)draw_item_value(5); }
    void set_led_value(bool draw = true) { 
        menu_items_[7].value = led_int_ ? "on" : "off"; if(draw)draw_item_value(7); }

    int vdac_ = 0;
    int ac_ = 0;
    int ar_ = 0;
    int trig_ = 0;
    bool dac_e_ = 0;
    bool led_int_ = 0;
};

EngineeringMenu::EngineeringMenu() : 
    SimpleValueMenu(make_menu_items(), "Engineering menu") 
{
    sync_values();
}

void EngineeringMenu::sync_values()
{
    unsigned all_gpio = gpio_get_all();
    vdac_ = all_gpio&0x0000FF;
    ar_ = (all_gpio>>8)&0x00000F;
    ac_ = (all_gpio>>12)&0x00000F;
    trig_ = (all_gpio>>19)&0x000001;
    dac_e_ = (all_gpio>>20)&0x000001;
    led_int_ = (all_gpio>>PICO_DEFAULT_LED_PIN)&0x000001;
    set_vdac_value(false);
    set_ac_value(false);
    set_ar_value(false);
    set_dac_e_value(false);
    set_trig_value(false);
    set_led_value(false);
}

bool EngineeringMenu::process_key_press(int key, int key_count, int& return_code)
{
    switch(key) {
    case '+':
        vdac_ = std::min(vdac_ + (key_count >= 15 ? 5 : 1), 255);
        gpio_put_masked(0x0000FF, vdac_ & 0x0000FF);
        set_vdac_value();
        break;
    case '-':
        vdac_ = std::max(vdac_ - (key_count >= 15 ? 5 : 1), 0);
        gpio_put_masked(0x0000FF, vdac_ & 0x0000FF);
        set_vdac_value();
        break;
    case 'Z':
        vdac_ = 0;
        gpio_put_masked(0x0000FF, vdac_ & 0x0000FF);
        set_vdac_value();
        break;
    case 'L':
        led_int_ = !led_int_;
        gpio_put(PICO_DEFAULT_LED_PIN, led_int_ ? 1 : 0);
        set_led_value();
        break;
    case 'K':
        {
            KeypressMenu key_menu;
            key_menu.event_loop();
            this->redraw();
        }
        break;
    case '\007':
        cls();
        curpos(1,1);
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
        break;
    }
    // char buffer[80];
    // if(isprint(key)) {
    //     sprintf(buffer,"%c %d %d        ",char(key),key,key_count);
    // } else {
    //     sprintf(buffer,"%c %d %d        ",' ',key,key_count);        
    // }
    // curpos(3,3);
    // puts_raw_nonl(buffer);
    return true;
}

bool EngineeringMenu::process_timeout(int& return_code)
{
    return true;
}

int main()
{
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0);

    gpio_init_mask(0x0018FFFF);
    gpio_set_dir_out_masked(0x0018FFFF);
    gpio_clr_mask(0x0018FFFF);

    stdio_init_all();

    EngineeringMenu menu;
    menu.event_loop();
}
