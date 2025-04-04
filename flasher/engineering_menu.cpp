#include <cmath>

#include <hardware/adc.h>

#include "flasher.hpp"
#include "menu.hpp"
#include "input_menu.hpp"
#include "engineering_menu.hpp"

namespace {
    static BuildDate build_date(__DATE__,__TIME__);
}

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
    dac_wr_  = (all_gpio >> DAC_WR_PIN)     & 0x000001;
    dac_sel_ = (all_gpio >> DAC_SEL_BASE_PIN) & 0x000003;
    spi_clk_ = (all_gpio >> SPI_CLK_PIN)    & 0x000001;
    spi_dout_ = (all_gpio >> SPI_DOUT_PIN)   & 0x000001;
    spi_col_en_ = (all_gpio >> SPI_COL_EN_PIN) & 0x000001;
    spi_all_en_ = (all_gpio >> SPI_ALL_EN_PIN) & 0x000001;
    set_vdac_value(false);
    set_rc_value(false);
    set_dac_e_value(false);
    set_trig_value(false);
    set_led_value(false);
    set_dac_wr_value(false);
    set_dac_sel_value(false);
    set_spi_clk_value(false);
    set_spi_dout_value(false);
    set_spi_col_en_value(false);
    set_spi_all_en_value(false);
}

void EngineeringMenu::set_vdac_value(bool draw) 
{ 
    menu_items_[MIP_VDAC].value = std::to_string(vdac_); 
    if(draw)draw_item_value(MIP_VDAC);
}

void EngineeringMenu::set_rc_value(bool draw) 
{ 
    rc_to_value_string(menu_items_[MIP_ROWCOL].value, ar_, ac_);
    if(draw)draw_item_value(MIP_ROWCOL);
}

void EngineeringMenu::set_dac_e_value(bool draw) 
{ 
    menu_items_[MIP_DAC_EN].value = dac_e_ ? ">ON<" : "off";
    menu_items_[MIP_DAC_EN].value_style = dac_e_ ? ANSI_INVERT : ""; 
    if(draw)draw_item_value(MIP_DAC_EN);
}

void EngineeringMenu::set_trig_value(bool draw) 
{ 
    menu_items_[MIP_TOGGLE_TRIG].value = trig_ ? ">ON<" : "off"; 
    menu_items_[MIP_TOGGLE_TRIG].value_style = trig_ ? ANSI_INVERT : "";
    if(draw)draw_item_value(MIP_TOGGLE_TRIG); 
}

void EngineeringMenu::set_led_value(bool draw) 
{ 
    menu_items_[MIP_LED].value = led_int_ ? ">ON<" : "off"; 
    menu_items_[MIP_LED].value_style = gpio_get(PICO_DEFAULT_LED_PIN) ? ANSI_INVERT : "";
    if(draw)draw_item_value(MIP_LED); 
}

void EngineeringMenu::set_dac_wr_value(bool draw) 
{ 
    menu_items_[MIP_DAC_WR].value = dac_wr_ ? ">ON<" : "off"; 
    menu_items_[MIP_DAC_WR].value_style = dac_wr_ ? ANSI_INVERT : "";
    if(draw)draw_item_value(MIP_DAC_WR); 
}

void EngineeringMenu::set_dac_sel_value(bool draw) 
{ 
    static const char* name[]= {"MAIN", "SCALE", "SPARE", "TRIM"};
    menu_items_[MIP_DAC_SEL].value = name[dac_sel_]; 
    if(draw)draw_item_value(MIP_DAC_SEL);
}

void EngineeringMenu::set_spi_clk_value(bool draw) 
{ 
    menu_items_[MIP_SPI_CLK].value = spi_clk_ ? ">ON<" : "off"; 
    menu_items_[MIP_SPI_CLK].value_style = spi_clk_ ? ANSI_INVERT : "";
    if(draw)draw_item_value(MIP_SPI_CLK); 
}

void EngineeringMenu::set_spi_dout_value(bool draw) 
{ 
    menu_items_[MIP_SPI_DOUT].value = spi_dout_ ? ">ON<" : "off"; 
    menu_items_[MIP_SPI_DOUT].value_style = spi_dout_ ? ANSI_INVERT : "";
    if(draw)draw_item_value(MIP_SPI_DOUT); 
}

void EngineeringMenu::set_spi_col_en_value(bool draw) 
{ 
    menu_items_[MIP_SPI_COL_EN].value = spi_col_en_ ? ">ON<" : "off"; 
    menu_items_[MIP_SPI_COL_EN].value_style = spi_col_en_ ? ANSI_INVERT : "";
    if(draw)draw_item_value(MIP_SPI_COL_EN); 
}

void EngineeringMenu::set_spi_all_en_value(bool draw) 
{ 
    menu_items_[MIP_SPI_ALL_EN].value = spi_all_en_ ? ">ON<" : "off"; 
    menu_items_[MIP_SPI_ALL_EN].value_style = spi_all_en_ ? ANSI_INVERT : "";
    if(draw)draw_item_value(MIP_SPI_ALL_EN); 
}

std::vector<SimpleItemValueMenu::MenuItem> EngineeringMenu::make_menu_items() 
{
    std::vector<SimpleItemValueMenu::MenuItem> menu_items(MIP_NUM_ITEMS);
    menu_items.at(MIP_ROWCOL)      = {"Cursors : Change column & row", 3, "A1"};

    menu_items.at(MIP_VDAC)        = {"</S/>   : Decrease/Set/Increase DAC setting", 3, "0"};
    menu_items.at(MIP_ZERO_VDAC)   = {"Z       : Zero DAC setting", 0, ""};
    menu_items.at(MIP_DAC_EN)      = {"V       : Toggle DAC voltage distribution", 4, "off"};
    menu_items.at(MIP_DAC_SEL)     = {"C       : Cycle between DACs", 5, "MAIN"};
    menu_items.at(MIP_DAC_WR)      = {"W       : Toggle DAC write enable", 4, "off"};

    menu_items.at(MIP_TOGGLE_TRIG) = {"T       : Toggle trigger", 4, "off"};
    menu_items.at(MIP_PULSE_TRIG)  = {"P       : Pulse trigger", 0, ""};

    menu_items.at(MIP_SPI_CLK)     = {"K       : Toggle SPI clock", 4, "off"};
    menu_items.at(MIP_SPI_DOUT)    = {"D       : Toggle SPI data out", 4, "off"};
    menu_items.at(MIP_SPI_COL_EN)  = {"R       : Toggle SPI row/col enable", 4, "off"};
    menu_items.at(MIP_SPI_ALL_EN)  = {"A       : Toggle SPI all enable", 4, "off"};

    menu_items.at(MIP_LED)         = {"L       : Toggle Raspberry-Pi Pico on-board LED", 4, "off"};
    menu_items.at(MIP_TEMPERATURE) = {"m       : Measure Pico on-board temperature", 4, ""};

    menu_items.at(MIP_EXIT)        = {"q       : Exit menu", 0, ""};
    return menu_items;
}

bool EngineeringMenu::process_key_press(int key, int key_count, int& return_code,
    const std::vector<std::string>& escape_sequence_parameters,
    absolute_time_t& next_timer)
{
    if(process_rc_keys(ar_, ac_, key, key_count)) {
        gpio_put_masked((0x00000F << ROW_A_BASE_PIN)|(0x00000F << COL_A_BASE_PIN),
                        (ar_ << ROW_A_BASE_PIN)|(ac_ << COL_A_BASE_PIN));
        set_rc_value();
        return true;
    }

    switch(key) {
    case '>':
        increase_value_in_range(vdac_, 255, (key_count >= 15 ? 5 : 1), key_count==1);
        gpio_put_masked(0x0000FF << VDAC_BASE_PIN, vdac_ << VDAC_BASE_PIN);
        set_vdac_value();
        break;
    case '<':
        decrease_value_in_range(vdac_, 0, (key_count >= 15 ? 5 : 1), key_count==1);
        gpio_put_masked(0x0000FF << VDAC_BASE_PIN, vdac_ << VDAC_BASE_PIN);
        set_vdac_value();
        break;
    case 'S':
        if(InplaceInputMenu::input_value_in_range(vdac_, 0, 255, this, MIP_VDAC, 3)) {
            gpio_put_masked(0x0000FF << VDAC_BASE_PIN, vdac_ << VDAC_BASE_PIN);
        }
        set_vdac_value(true);
        break;
    case 'Z':
        vdac_ = 0;
        gpio_put_masked(0x0000FF << VDAC_BASE_PIN, vdac_ << VDAC_BASE_PIN);
        set_vdac_value();
        break;
    case 'V':
        dac_e_ = !dac_e_;
        gpio_put(DAC_EN_PIN, dac_e_ ? 1 : 0);
        set_dac_e_value();
        break;
    case 'W':
        dac_wr_ = !dac_wr_;
        gpio_put(DAC_WR_PIN, dac_wr_ ? 1 : 0);
        set_dac_wr_value();
        break;
    case 'C':
    case 3: // ctrl-c
        dac_wr_ = false; // make sure DAC_WR is OFF when user changes selected DAC
        gpio_put(DAC_WR_PIN, 0);
        set_dac_wr_value();
        dac_sel_ = (dac_sel_+1) % 4;
        if(key!=3 and dac_sel_== 2) {
            dac_sel_ = 3;
        }
        gpio_put_masked(0x000003 << DAC_SEL_BASE_PIN, dac_sel_ << DAC_SEL_BASE_PIN);
        set_dac_sel_value();
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

    case 'K':
        spi_clk_ = !spi_clk_;
        gpio_put(SPI_CLK_PIN, spi_clk_ ? 1 : 0);
        set_spi_clk_value();
        break;
    case 'D':
        spi_dout_ = !spi_dout_;
        gpio_put(SPI_DOUT_PIN, spi_dout_ ? 1 : 0);
        set_spi_dout_value();
        break;
    case 'R':
        spi_col_en_ = !spi_col_en_;
        gpio_put(SPI_COL_EN_PIN, spi_col_en_ ? 1 : 0);
        set_spi_col_en_value();
        break;
    case 'A':
        spi_all_en_ = !spi_all_en_;
        gpio_put(SPI_ALL_EN_PIN, spi_all_en_ ? 1 : 0);
        set_spi_all_en_value();
        break;

    case 'L':
        led_int_ = !led_int_;
        gpio_put(PICO_DEFAULT_LED_PIN, led_int_ ? 1 : 0);
        set_led_value();
        break;
    case 'm':
        {
            const float conversion_factor = 3.3f / (1 << 12);
            adc_select_input(4);
            uint16_t result = adc_read();
            float voltage = result * conversion_factor;
            float temperature = 27.0f - (voltage - 0.706f)/0.001721f;
            // float temperature = floor((27.0 - (voltage - 0.706f) / 0.001721f) * 10 + 0.5) * 0.1;
            menu_items_[MIP_TEMPERATURE].value = std::to_string(temperature); 
            draw_item_value(MIP_TEMPERATURE); 
        }
        break
    case 'q':
    case 'Q':
        return_code = 0;
        return false;

    default:
        if(key_count==1) {
            beep();
        }
    }

    return true;
}

bool EngineeringMenu::process_timer(bool controller_is_connected, int& return_code, 
    absolute_time_t& next_timer)
{
    heartbeat_timer_count_ += 1;
    if(heartbeat_timer_count_ == 100) {
        if(controller_is_connected) {
            set_heartbeat(!heartbeat_);
        }
        heartbeat_timer_count_ = 0;
    }

    return true;
}
