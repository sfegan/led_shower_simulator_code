#pragma once

#include <vector>

#include <pico/stdlib.h>

#include "flasher.hpp"
#include "menu.hpp"

class EngineeringMenu: public SimpleItemValueMenu {
public:
    EngineeringMenu();
    virtual ~EngineeringMenu() { }
    bool controller_connected(int& return_code) final;
    bool controller_disconnected(int& return_code) final;
    bool process_key_press(int key, int key_count, int& return_code,
        const std::vector<std::string>& escape_sequence_parameters) final;
    bool process_timer(bool controller_is_connected, int& return_code) final;

private:
    enum MenuItemPositions {
        MIP_ROWCOL,
        MIP_VDAC,
        MIP_ZERO_VDAC,
        MIP_DAC_EN,
        MIP_DAC_SEL,
        MIP_DAC_WR,
        MIP_TOGGLE_TRIG,
        MIP_PULSE_TRIG,
        MIP_SPI_CLK,
        MIP_SPI_DOUT,
        MIP_SPI_COL_EN,
        MIP_SPI_ALL_EN,
        MIP_LED,
        MIP_EXIT,
        MIP_NUM_ITEMS // MUST BE LAST ITEM IN LIST
    };

    static std::vector<MenuItem> make_menu_items() {
        std::vector<MenuItem> menu_items(MIP_NUM_ITEMS);
        menu_items.at(MIP_ROWCOL)      = {"Cursors : Change column & row", 3, "A1"};

        menu_items.at(MIP_VDAC)        = {"</>     : Increase/decrease DAC voltage", 3, "0"};
        menu_items.at(MIP_ZERO_VDAC)   = {"Z       : Zero DAC voltage", 0, ""};
        menu_items.at(MIP_DAC_EN)      = {"V       : Toggle DAC voltage distribution", 4, "off"};
        menu_items.at(MIP_DAC_SEL)     = {"S       : Select DAC", 5, "MAIN"};
        menu_items.at(MIP_DAC_WR)      = {"W       : Toggle DAC write enable", 4, "off"};

        menu_items.at(MIP_TOGGLE_TRIG) = {"T       : Toggle trigger", 4, "off"};
        menu_items.at(MIP_PULSE_TRIG)  = {"P       : Pulse trigger", 0, ""};

        menu_items.at(MIP_SPI_CLK)     = {"C       : Toggle SPI clock", 4, "off"};
        menu_items.at(MIP_SPI_DOUT)    = {"D       : Toggle SPI data out", 4, "off"};
        menu_items.at(MIP_SPI_COL_EN)  = {"R       : Toggle SPI row/col enable", 4, "off"};
        menu_items.at(MIP_SPI_ALL_EN)  = {"A       : Toggle SPI all enable", 4, "off"};

        menu_items.at(MIP_LED)         = {"L       : Toggle on-board LED", 4, "off"};
        menu_items.at(MIP_EXIT)        = {"q       : Exit menu", 0, ""};
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
        menu_items_[MIP_LED].value_style = gpio_get(PICO_DEFAULT_LED_PIN) ? ANSI_INVERT : "";
        if(draw)draw_item_value(MIP_LED); 
    }
    void set_dac_wr_value(bool draw = true) { 
        menu_items_[MIP_DAC_WR].value = dac_wr_ ? ">ON<" : "off"; 
        menu_items_[MIP_DAC_WR].value_style = dac_wr_ ? ANSI_INVERT : "";
        //std::to_string(dac_wr_);
        if(draw)draw_item_value(MIP_DAC_WR); 
    }
    void set_dac_sel_value(bool draw = true) { 
        static const char* name[]= {"MAIN", "SCALE", "SPARE", "TRIM"};
        menu_items_[MIP_DAC_SEL].value = name[dac_sel_]; 
        if(draw)draw_item_value(MIP_DAC_SEL);
    }
    void set_spi_clk_value(bool draw = true) { 
        menu_items_[MIP_SPI_CLK].value = spi_clk_ ? ">ON<" : "off"; 
        menu_items_[MIP_SPI_CLK].value_style = spi_clk_ ? ANSI_INVERT : "";
        if(draw)draw_item_value(MIP_SPI_CLK); 
    }
    void set_spi_dout_value(bool draw = true) { 
        menu_items_[MIP_SPI_DOUT].value = spi_dout_ ? ">ON<" : "off"; 
        menu_items_[MIP_SPI_DOUT].value_style = spi_dout_ ? ANSI_INVERT : "";
        if(draw)draw_item_value(MIP_SPI_DOUT); 
    }
    void set_spi_col_en_value(bool draw = true) { 
        menu_items_[MIP_SPI_COL_EN].value = spi_col_en_ ? ">ON<" : "off"; 
        menu_items_[MIP_SPI_COL_EN].value_style = spi_col_en_ ? ANSI_INVERT : "";
        if(draw)draw_item_value(MIP_SPI_COL_EN); 
    }
    void set_spi_all_en_value(bool draw = true) { 
        menu_items_[MIP_SPI_ALL_EN].value = spi_all_en_ ? ">ON<" : "off"; 
        menu_items_[MIP_SPI_ALL_EN].value_style = spi_all_en_ ? ANSI_INVERT : "";
        if(draw)draw_item_value(MIP_SPI_ALL_EN); 
    }
   
    int vdac_ = 0;
    int ac_ = 0;
    int ar_ = 0;
    bool trig_ = 0;
    bool dac_e_ = 0;
    bool led_int_ = 0;
    int dac_wr_ = 0;
    int dac_sel_ = 0;
    int spi_clk_ = 0;
    int spi_dout_ = 0;
    int spi_col_en_ = 0;
    int spi_all_en_ = 0;
    unsigned timer_count_ = 0;
};
