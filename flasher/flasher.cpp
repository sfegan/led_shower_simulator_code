#include <string>
#include <vector>

#include <cstring>
#include <cstdio>
#include <cctype>

#include <pico/stdlib.h>

#include "menu.hpp"
#include "event_generators.hpp"
#include "event_dispatcher.hpp"

enum Pins { VDAC_BASE_PIN       = 0,
            ROW_A_BASE_PIN      = 8, 
            COL_A_BASE_PIN      = 12,
            DAC_EN_PIN          = 16,
            TRIG_PIN            = 17,
            SPI_CLK_PIN         = 18,
            SPI_DOUT_PIN        = 19,
            SPI_COL_EN_PIN      = 20,
            SPI_ALL_EN_PIN      = 21,
            SPARE_PIN           = 22,
            DAC_WR_PIN          = 26,
            DAC_SEL_BASE_PIN    = 27 };

class KeypressMenu: public Menu {
public:
    virtual ~KeypressMenu() { }
    void redraw() override;
    bool process_key_press(int key, int key_count, int& return_code, 
        const std::vector<std::string>& escape_sequence_parameters) override;
    bool process_timeout(int& return_code) override;
};

void KeypressMenu::redraw()
{
    cls();
    curpos(1,1);
    puts("Type some keys (terminate with Ctrl-D)");
}

bool KeypressMenu::process_key_press(int key, int key_count, int& return_code,
    const std::vector<std::string>& escape_sequence_parameters)
{
    char buffer[80];
    sprintf(buffer, "%c %d \\%o %d",(key<256 and isprint(key))?key:' ',key,key,key_count);
    if(escape_sequence_parameters.empty()) {
        puts(buffer);    
    } else {
        puts_raw_nonl(buffer);
        puts_raw_nonl(" (");
        for(unsigned i=0; i<escape_sequence_parameters.size(); ++i) {
            if(i!=0)puts_raw_nonl(", ");
            puts_raw_nonl(escape_sequence_parameters[i]);
        }
        puts(")");
    }
    return_code = 0;
    if(key == '\003') {
        send_request_screen_size();
    } else if(key == '\010') {
        hide_cursor();
    } else if(key == '\011') {
        show_cursor();
    }
    return key != '\004';
}

bool KeypressMenu::process_timeout(int& return_code)
{
    return_code = 0;
    return true;
}

#define WRITEVAL(x) \
    { \
        char buffer[80]; \
        sprintf(buffer, "%-20s : %d\n\r", #x, x); \
        puts_raw_nonl(buffer); \
    }


class EngineeringMenu: public SimpleItemValueMenu {
public:
    EngineeringMenu();
    virtual ~EngineeringMenu() { }
    bool process_key_press(int key, int key_count, int& return_code, 
        const std::vector<std::string>& escape_sequence_parameters) final;
    bool process_timeout(int& return_code) final;

private:
    enum MenuItemPositions {
        MIP_VDAC       = 0,
        MIP_ZERO_VDAC,
        MIP_ROWCOL,
        MIP_DAC_EN,
        MIP_TOGGLE_TRIG,
        MIP_PULSE_TRIG,
        MIP_LED,
        MIP_KEYPRESS,
        MIP_NUM_ITEMS // MUST BE LAST ITEM IN LIST
    };

    static std::vector<MenuItem> make_menu_items() {
        std::vector<MenuItem> menu_items(MIP_NUM_ITEMS);
        menu_items.at(MIP_VDAC)        = {"</>     : Increase/decrease DAC voltage", 3, "0"};
        menu_items.at(MIP_ZERO_VDAC)   = {"Z       : Zero DAC voltage", 0, ""};
        menu_items.at(MIP_ROWCOL)      = {"Cursors : Change column & row", 3, "A1"};
        menu_items.at(MIP_DAC_EN)      = {"D       : Toggle DAC enabled", 4, "off"};
        menu_items.at(MIP_TOGGLE_TRIG) = {"T       : Toggle trigger", 4, "off"};
        menu_items.at(MIP_PULSE_TRIG)  = {"P       : Pulse trigger", 0, ""};
        menu_items.at(MIP_LED)         = {"L       : Toggle on-board LED", 4, "off"};
        menu_items.at(MIP_KEYPRESS)    = {"k       : Display keypress", 0, ""};
        return menu_items;
    }

    void sync_values();
    void set_vdac_value(bool draw = true) { 
        menu_items_[MIP_VDAC].value = std::to_string(vdac_); 
        if(draw)draw_item_value(MIP_VDAC);
    }
    void set_rc_value(bool draw = true) { 
        menu_items_[MIP_ROWCOL].value = std::string(1, char('A' + ar_)) 
            + std::to_string(ac_); 
        if(draw)draw_item_value(MIP_ROWCOL);
    }
    void set_dac_e_value(bool draw = true) { 
        menu_items_[MIP_DAC_EN].value = dac_e_ ? ">ON<" : "off";
        menu_items_[MIP_DAC_EN].value_style = dac_e_ ? ANSI_INVERT : ""; 
        if(draw)draw_item_value(MIP_DAC_EN);
    }
    void set_trig_value(bool draw = true) { 
        menu_items_[MIP_TOGGLE_TRIG].value = trig_ ? ">ON<" : "off"; 
        menu_items_[MIP_TOGGLE_TRIG].value_style = trig_ ? ANSI_INVERT : "";
        if(draw)draw_item_value(MIP_TOGGLE_TRIG); 
    }
    void set_led_value(bool draw = true) { 
        menu_items_[MIP_LED].value = led_int_ ? ">ON<" : "off"; 
        menu_items_[MIP_LED].value_style = led_int_ ? ANSI_INVERT : "";
        if(draw)draw_item_value(MIP_LED); 
    }

    int vdac_ = 0;
    int ac_ = 0;
    int ar_ = 0;
    bool trig_ = 0;
    bool dac_e_ = 0;
    bool led_int_ = 0;
};

EngineeringMenu::EngineeringMenu() : 
    SimpleItemValueMenu(make_menu_items(), "Engineering menu") 
{
    sync_values();
}

void EngineeringMenu::sync_values()
{
    unsigned all_gpio = gpio_get_all();
    vdac_    = (all_gpio >> VDAC_BASE_PIN)  & 0x0000FF;
    ar_      = (all_gpio >> ROW_A_BASE_PIN) & 0x00000F;
    ac_      = (all_gpio >> COL_A_BASE_PIN) & 0x00000F;
    trig_    = (all_gpio >> TRIG_PIN)       & 0x000001;
    dac_e_   = (all_gpio >> DAC_EN_PIN)     & 0x000001;
    led_int_ = (all_gpio >> PICO_DEFAULT_LED_PIN) & 0x000001;
    set_vdac_value(false);
    set_rc_value(false);
    set_dac_e_value(false);
    set_trig_value(false);
    set_led_value(false);
}

bool EngineeringMenu::process_key_press(int key, int key_count, int& return_code,
    const std::vector<std::string>& escape_sequence_parameters)
{
    switch(key) {
    case '>':
    case '+':
        vdac_ = std::min(vdac_ + (key_count >= 15 ? 5 : 1), 255);
        gpio_put_masked(0x0000FF << VDAC_BASE_PIN, vdac_ << VDAC_BASE_PIN);
        set_vdac_value();
        break;
    case '<':
    case '-':
        vdac_ = std::max(vdac_ - (key_count >= 15 ? 5 : 1), 0);
        gpio_put_masked(0x0000FF << VDAC_BASE_PIN, vdac_ << VDAC_BASE_PIN);
        set_vdac_value();
        break;
    case 'Z':
        vdac_ = 0;
        gpio_put_masked(0x0000FF << VDAC_BASE_PIN, vdac_ << VDAC_BASE_PIN);
        set_vdac_value();
        break;
    case KEY_UP:
        ar_ = std::max(ar_-1, 0);
        gpio_put_masked(0x00000F << ROW_A_BASE_PIN, ar_ << ROW_A_BASE_PIN);
        set_rc_value();
        break;
    case KEY_DOWN:
        ar_ = std::min(ar_+1, 15);
        gpio_put_masked(0x00000F << ROW_A_BASE_PIN, ar_ << ROW_A_BASE_PIN);
        set_rc_value();
        break;
    case KEY_LEFT:
        ac_ = std::max(ac_-1, 0);
        gpio_put_masked(0x00000F << COL_A_BASE_PIN, ac_ << COL_A_BASE_PIN);
        set_rc_value();
        break;
    case KEY_RIGHT:
        ac_ = std::min(ac_+1, 15);
        gpio_put_masked(0x00000F << COL_A_BASE_PIN, ac_ << COL_A_BASE_PIN);
        set_rc_value();
        break;
    case KEY_PAGE_UP:
        ar_ = 0;
        gpio_put_masked(0x00000F << ROW_A_BASE_PIN, ar_ << ROW_A_BASE_PIN);
        set_rc_value();
        break;
    case KEY_PAGE_DOWN:
        ar_ = 15;
        gpio_put_masked(0x00000F << ROW_A_BASE_PIN, ar_ << ROW_A_BASE_PIN);
        set_rc_value();
        break;
    case KEY_HOME:
        ac_ = 0;
        gpio_put_masked(0x00000F << COL_A_BASE_PIN, ac_ << COL_A_BASE_PIN);
        set_rc_value();
        break;
    case KEY_END:
        ac_ = 15;
        gpio_put_masked(0x00000F << COL_A_BASE_PIN, ac_ << COL_A_BASE_PIN);
        set_rc_value();
        break;
    case 'D':
        dac_e_ = !dac_e_;
        gpio_put(DAC_EN_PIN, dac_e_ ? 1 : 0);
        set_dac_e_value();
        break;
    case 'T':
        trig_ = !trig_;
        gpio_put(TRIG_PIN, trig_ ? 1 : 0);
        set_trig_value();
        break;
    case 'P':
        gpio_put(TRIG_PIN, 1);
        gpio_put(TRIG_PIN, 0);
        trig_ = 1;
        set_trig_value();
        sleep_ms(100);
        trig_ = 0;
        set_trig_value();
        break;
    case 'L':
        led_int_ = !led_int_;
        gpio_put(PICO_DEFAULT_LED_PIN, led_int_ ? 1 : 0);
        set_led_value();
        break;
    case 'k':
    case 'K':
    case 11: // ctrl-K
        {
            KeypressMenu key_menu;
            key_menu.event_loop(key != 11);
            this->redraw();
        }
        break;
    case '\007':
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
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);

    uint32_t pin_mask =
        (0xFFU << VDAC_BASE_PIN) 
        | (0xFU << ROW_A_BASE_PIN)
        | (0xFU << COL_A_BASE_PIN)
        | (0x1U << DAC_EN_PIN)
        | (0x1U << TRIG_PIN)
        | (0x1U << SPI_CLK_PIN)
        | (0x1U << SPI_DOUT_PIN)
        | (0x1U << SPI_COL_EN_PIN)
        | (0x1U << SPI_ALL_EN_PIN)
        | (0x1U << DAC_WR_PIN)
        | (0x3U << DAC_SEL_BASE_PIN);

    gpio_init_mask(pin_mask);
    gpio_clr_mask(pin_mask);
    gpio_set_dir_out_masked(pin_mask);

    stdio_init_all();

    // EventDispatcher::instance().start_dispatcher();

    EngineeringMenu menu;
    // SingleLEDEventGenerator menu;
    // EventDispatcher::instance().register_event_generator(&menu);
    menu.event_loop();
}
