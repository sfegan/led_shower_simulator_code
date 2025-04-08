#pragma once

#include <vector>

#include <pico/stdlib.h>

#include "flasher.hpp"
#include "menu.hpp"

class EngineeringMenu: public SimpleItemValueMenu {
public:
    EngineeringMenu();
    virtual ~EngineeringMenu() { }
    bool process_key_press(int key, int key_count, int& return_code,
        const std::vector<std::string>& escape_sequence_parameters, absolute_time_t& next_timer) final;
    bool process_timer(bool controller_is_connected, int& return_code, absolute_time_t& next_timer) final;

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
        MIP_TEMPERATURE,
        MIP_EXIT,
        MIP_NUM_ITEMS // MUST BE LAST ITEM IN LIST
    };

    std::vector<MenuItem> make_menu_items();

    void sync_values();
    void set_vdac_value(bool draw = true);
    void set_rc_value(bool draw = true);
    void set_dac_e_value(bool draw = true);
    void set_trig_value(bool draw = true);
    void set_led_value(bool draw = true);
    void set_led_timer_count_value(bool draw = true); 
    void set_dac_wr_value(bool draw = true);
    void set_dac_sel_value(bool draw = true);
    void set_spi_clk_value(bool draw = true);
    void set_spi_dout_value(bool draw = true);
    void set_spi_col_en_value(bool draw = true);
    void set_spi_all_en_value(bool draw = true);
    void set_measured_temp_value(bool draw = true);

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
    unsigned heartbeat_timer_count_ = 0;
    bool measure_temp_ = false;
};
