#include "flasher.hpp"
#include "menu.hpp"
#include "engineering_menu.hpp"

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

bool EngineeringMenu::controller_connected(int& return_code)
{
    return_code = 0;
    return true;
}

bool EngineeringMenu::controller_disconnected(int& return_code)
{
    return_code = 0;
    return true;
}

bool EngineeringMenu::process_key_press(int key, int key_count, int& return_code,
    const std::vector<std::string>& escape_sequence_parameters)
{
    switch(key) {
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
    case 'S':
    case 19: // ctrl-s
        dac_wr_ = false; // make sure DAC_WR is OFF when user changes selected DAC
        gpio_put(DAC_WR_PIN, 0);
        set_dac_wr_value();
        dac_sel_ = (dac_sel_+1) % 4;
        if(key=='S' and dac_sel_== 2) {
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

    case 'C':
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
        timer_count_ = 0;
        set_led_value();
        break;

    case 'q':
    case 'Q':
        return_code = 0;
        return false;
    }

    return true;
}

bool EngineeringMenu::process_timer(bool controller_is_connected, int& return_code)
{
    timer_count_ += 1;
    if(timer_count_ == 100) {
        if(led_int_) {
            gpio_put(PICO_DEFAULT_LED_PIN, !gpio_get(PICO_DEFAULT_LED_PIN));
            set_led_value(true);
        }
        timer_count_ = 0;
    }
    return_code = 0;
    return true;
}
