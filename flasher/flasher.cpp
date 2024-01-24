#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <pico/stdlib.h>

class Menu {
public:
    virtual ~Menu() { }
    virtual void redraw() = 0;
    virtual bool process_key_press(int key, int key_count, int& return_code) = 0;
    virtual bool process_timeout(int& return_code) = 0;

    static void cls() { puts("\027[2J\027[1;1H"); }
    static void curpos(inr r, int c) { 
        char buffer[80]; 
        sprintf("\027[%d;%dJ",r,c); 
        puts(buffer);
    }
};

class EngineeringMenu: public Menu {
public:
    virtual ~EngineeringMenu() { }
    void redraw() final;
    bool process_key_press(int key, int key_count, int& return_code) final;
    bool process_timeout(int& return_code) final;
private:
    int vdac_ = 0;
    int ac_ = 0;
    int ar_ = 0;
    int trig_ = 0;
    bool dac_e_ = 0;
};

EngineeringMenu::redraw()
{

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

}
