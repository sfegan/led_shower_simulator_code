#include <cstdio>

#include "flasher.hpp"
#include "keypress_menu.hpp"

namespace {
    static BuildDate build_date(__DATE__,__TIME__);
}

void KeypressMenu::redraw()
{
    cls();
    curpos(1,1);
    puts("Type some keys (terminate with Ctrl-d)");
}

bool KeypressMenu::controller_connected(int& return_code)
{
    return_code = 0;
    return true;
}


bool KeypressMenu::controller_disconnected(int& return_code)
{
    return_code = 0;
    return true;
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

bool KeypressMenu::process_timer(bool controller_is_connected, int& return_code)
{
    return_code = 0;
    return true;
}
