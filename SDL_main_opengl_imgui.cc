#pragma once

#include <GL/glew.h>
#include <SDL2/SDL.h> 
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_timer.h> 
#include <time.h>


#include "import/imgui/imgui_draw.cpp"
#include "import/imgui/imgui_widgets.cpp"
#include "import/imgui/imgui.cpp"
#include "import/imgui/imgui_impl_sdl.cpp"
#include "import/imgui/imgui_impl_opengl3.cpp"

#include "cp_lib/basic.cc"
#include "cp_lib/vector.cc"

#include "game.cc"

using namespace cp;

void gl_debug_callback(GLenum source,
            GLenum type, GLuint id,
            GLenum severity, GLsizei length,
            const GLchar *message, const void *userParam) 
{
    puts(message);
}

int main()
{

    window_size = {1280, 720};

    SDL_Init(SDL_INIT_EVERYTHING);

    window = SDL_CreateWindow("GPU Graphics", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_size.x, window_size.y, SDL_WINDOW_OPENGL);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GLContext gr_context = SDL_GL_CreateContext(window);

    if (glewInit() != GLEW_OK) {
        puts("Glew init failed!");
    }

    // enable debug stuff
    puts((char*)glGetString(GL_VERSION));

    glEnable(GL_DEBUG_OUTPUT);
    // glDebugMessageCallback(gl_debug_callback, null);
    //
    
    // gui init

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForOpenGL(window, &gr_context);
    ImGui_ImplOpenGL3_Init("#version 440 core");
    //


    game_init();

    // clock_t pre_clock = clock();
    u32 pre_clock = SDL_GetTicks();
    while (is_running) {
        for (u32 i = 0; i < Input::keys_down.cap; i++) {
            Input::keys_down.buffer[i] = 0;
            Input::keys_up.buffer[i] = 0;
        }
        for (u32 i = 0; i < bitfld_byte_count(MOUSE_BUTTON_COUNT); i++) {
            Input::mouse_buttons_down.buffer[i] = 0;
            Input::mouse_buttons_up.buffer[i] = 0;
        }
        Input::mouse_wheel = {};


        // Process events.
        SDL_Event event;
        while(SDL_PollEvent(&event)) 
        {
            // gui events
            ImGui_ImplSDL2_ProcessEvent(&event);

            switch(event.type)
            {
                case SDL_QUIT: is_running = false; break;

                case SDL_WINDOWEVENT: {
                    switch (event.window.event) {
                        case SDL_WINDOWEVENT_RESIZED: {
                            window_size = {event.window.data1, event.window.data2};
                            glViewport(0, 0, window_size.x, window_size.y);
                        } break;
                        case SDL_WINDOWEVENT_SIZE_CHANGED: {
                            window_size = {event.window.data1, event.window.data2};
                            glViewport(0, 0, window_size.x, window_size.y);
                        } break;
                        // case SDL_WINDOWEVENT_MINIMIZED:
                        //     printf("Window %d minimized", event.window.windowID);
                        //     break;
                        // case SDL_WINDOWEVENT_MAXIMIZED:
                        //     printf("Window %d maximized", event.window.windowID);
                        //     break;
                        // case SDL_WINDOWEVENT_RESTORED:
                        //     printf("Window %d restored", event.window.windowID);
                        //     break;
                    }
                } break;


                // input handling
                case SDL_KEYDOWN: {
                    if (event.key.keysym.sym < KEY_COUNT) {
                        set_bit_high(Input::keys_down, event.key.keysym.sym);
                        set_bit_high(Input::keys_hold, event.key.keysym.sym);
                    }
                } break;
                case SDL_KEYUP: {
                    if (event.key.keysym.sym < KEY_COUNT) {
                        set_bit_high(Input::keys_up, event.key.keysym.sym);
                        set_bit_low(Input::keys_hold, event.key.keysym.sym);
                    }
                } break;
                case SDL_MOUSEBUTTONDOWN: {
                    u8 button = event.button.button - 1;
                    if (button < MOUSE_BUTTON_COUNT) {
                        set_bit_high(Input::mouse_buttons_down, button);
                        set_bit_high(Input::mouse_buttons_hold, button);
                    }
                } break;
                case SDL_MOUSEBUTTONUP: {
                    u8 button = event.button.button - 1;
                    if (button < MOUSE_BUTTON_COUNT) {
                        set_bit_high(Input::mouse_buttons_up, button);
                        set_bit_low(Input::mouse_buttons_hold, button);
                    }
                } break;
                case SDL_MOUSEMOTION: {
                    Input::mouse_position = { event.motion.x, event.motion.y };
                } break;
                case SDL_MOUSEWHEEL: {
                    Input::mouse_wheel = { event.wheel.x, event.wheel.y };
                } break;
            }
        }

        // clock_t new_clock = clock();
        // GTime::dt = (f32)(new_clock - pre_clock) / CLOCKS_PER_SEC;
        // pre_clock = new_clock;

        // wall time
        u32 new_clock = SDL_GetTicks();
        GTime::dt = (f32)(new_clock - pre_clock) / 1000;
        pre_clock = new_clock;

        // render gui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();        

        game_update();
        // game_fixed_update();
        draw_gui();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        //

        SDL_GL_SwapWindow(window);
        SDL_Delay(1000/60);
    }

    game_shut();

    // gui cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gr_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
