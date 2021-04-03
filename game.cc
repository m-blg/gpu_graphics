#pragma once

#include "cp_lib/basic.cc"
#include "cp_lib/array.cc"
#include "cp_lib/vector.cc"
#include "cp_lib/bitfield.cc"
#include "cp_lib/memory.cc"

using namespace cp;


bool is_running = true;

vec2i window_max_size;
vec2i window_size;

dbuff2u frame_buffer;

namespace GTime {
    f32 dt;
    f32 fixed_dt;
}

// input
#define KEY_COUNT 256
#define MOUSE_BUTTON_COUNT 5

namespace Input {
    dbitfld keys_down;
    dbitfld keys_hold;
    dbitfld keys_up;

    dbitfld mouse_buttons_down;
    dbitfld mouse_buttons_hold;
    dbitfld mouse_buttons_up;

    vec2i mouse_position;

    vec2i mouse_wheel;


    void input_init() {
        Input::keys_down.init_bits(KEY_COUNT);
        Input::keys_hold.init_bits(KEY_COUNT);
        Input::keys_up.init_bits(KEY_COUNT);

        for (u32 i = 0; i < bitfld_byte_count(KEY_COUNT); i++) {
            keys_down.buffer[i] = 0;           
            keys_hold.buffer[i] = 0;           
            keys_up.buffer[i] = 0;       
        }

        Input::mouse_buttons_down.init_bits(MOUSE_BUTTON_COUNT);
        Input::mouse_buttons_hold.init_bits(MOUSE_BUTTON_COUNT);
        Input::mouse_buttons_up.init_bits(MOUSE_BUTTON_COUNT);
        
        for (u32 i = 0; i < bitfld_byte_count(MOUSE_BUTTON_COUNT); i++) {
            mouse_buttons_down.buffer[i] = 0;           
            mouse_buttons_hold.buffer[i] = 0;           
            mouse_buttons_up.buffer[i] = 0;       
        }

        mouse_position = {};

        mouse_wheel = {};
    }


    void input_shut() {
        Input::keys_down.shut();
        Input::keys_hold.shut();
        Input::keys_up.shut();

        Input::mouse_buttons_down.shut();
        Input::mouse_buttons_hold.shut();
        Input::mouse_buttons_up.shut();
    }

    bool is_key_down(u8 key) {
        return get_bit(Input::keys_down, key);
    }
    bool is_key_held(u8 key) {
        return get_bit(Input::keys_hold, key);
    }
    bool is_key_up(u8 key) {
        return get_bit(Input::keys_up, key);
    }

    bool is_mouse_button_down(u8 mouse_button) {
        return get_bit(Input::mouse_buttons_down, mouse_button);
    }
    bool is_mouse_button_held(u8 mouse_button) {
        return get_bit(Input::mouse_buttons_hold, mouse_button);
    }
    bool is_mouse_button_up(u8 mouse_button) {
        return get_bit(Input::mouse_buttons_up, mouse_button);
    }


};

void load_assets();


void game_init();
void game_shut();

void game_update();
void draw_gui();
