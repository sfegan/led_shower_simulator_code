#pragma once

#include <string>

#include "menu.hpp"

class InplaceInputMenu: public Menu {
public:
    enum ValidInput { STRING, FLOAT, POSITIVE_FLOAT, INTEGER, NATURAL };
    InplaceInputMenu(int r, int c, unsigned max_value_size, ValidInput valid_input=STRING, 
        bool do_highlight=true, Menu* base_menu = nullptr);
    virtual ~InplaceInputMenu();
    void redraw() override;
    bool controller_connected(int& return_code) override;
    bool controller_disconnected(int& return_code) override;
    bool process_key_press(int key, int key_count, int& return_code,
        const std::vector<std::string>& escape_sequence_parameters) override;
    bool process_timer(bool controller_is_connected, int& return_code) override;
    const std::string get_value() const { return value_; }
    void cancelled();
private:
    void draw_value();
    bool is_valid(int key);

    Menu* base_menu_ = nullptr;
    int r_;
    int c_;
    unsigned max_value_size_;
    ValidInput valid_input_ = STRING;
    std::string value_;
    bool do_highlight_;
    bool blink_on_ = false;
};

class InputMenu: public FramedMenu {
public:
    enum ValidInput { STRING, FLOAT, POSITIVE_FLOAT, INTEGER, NATURAL };
    InputMenu(unsigned max_value_size, ValidInput valid_input=STRING, const std::string title = "Enter value", 
        const std::string prompt = "Enter value: ", Menu* base_menu = nullptr);
    InputMenu(unsigned max_value_size, const std::string title,
        const std::string prompt = "Enter value: ", ValidInput valid_input=STRING, Menu* base_menu = nullptr);
    virtual ~InputMenu();
    void redraw() override;
    bool controller_connected(int& return_code) override;
    bool controller_disconnected(int& return_code) override;
    bool process_key_press(int key, int key_count, int& return_code,
        const std::vector<std::string>& escape_sequence_parameters) override;
    bool process_timer(bool controller_is_connected, int& return_code) override;
    const std::string get_value() const { return value_; }
private:
    void draw_value();
    bool is_valid(int key);

    Menu* base_menu_ = nullptr;
    unsigned max_value_size_;
    ValidInput valid_input_ = STRING;
    std::string value_;
    std::string prompt_;
    bool blink_on_ = false;
};
