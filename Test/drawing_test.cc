
#include "../SDL_main_opengl.cc"
#include "../loadings.cc"

#include "cp_lib/basic.cc"
#include "cp_lib/array.cc"
#include "cp_lib/vector.cc"
#include "cp_lib/memory.cc"
#include "cp_lib/io.cc"
#include <stdlib.h>
#include <unistd.h>

struct Triangle_Vertex {
    vec2f pos;
    vec4f color;
};

sbuff<Triangle_Vertex, 4> triangle_vertices = {{
    {{-0.5, -0.5}, {1, 0, 1, 0}},
    {{-0.5,  0.5}, {1, 0, 0, 1} },
    {{ 0.5, -0.5}, {1, 0, 1, 0}},
    {{ 0.5,  0.5}, {1, 1, 0, 0} }, 
}};

sbuff<u32[3], 2> triangle_indexes = {{
    {0, 1, 3},
    {0, 2, 3}
}};

float triangle_color[3][4] = {{1, 1, 0, 0}, {1, 0, 0, 1}, {1, 0, 1, 0}};
dbuff2f triangle_color_buffer = {(f32*)triangle_color, 3, 4};

u32 triangle_buffer_id;



void game_init() {
    Input::input_init();
    Assets::load_shaders<1>({"Test/test_shader.glsl"});
    window_size = {1280, 720};

    u32 vao_id;
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);

    // make buffer for triangle
    glGenBuffers(1, &triangle_buffer_id);
    glBindBuffer(GL_ARRAY_BUFFER, triangle_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_vertices), triangle_vertices.buffer, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Triangle_Vertex), 0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Triangle_Vertex), (void*)sizeof(vec2f));

    u32 ibo_id;
    glGenBuffers(1, &ibo_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triangle_indexes), triangle_indexes.buffer, GL_STATIC_DRAW);


    glUseProgram(Assets::shaders[0]);

    glClearColor(0, 0.2, 0.2, 1);

}

void game_shut() {
    Input::input_shut();
}


void game_update() {


    if (get_bit(Input::keys_down, 'q')) {
        is_running = false;
    }

    // triangle_vertices[0].pos = { ((f32)Input::mouse_pos.x / window_size.x) * 2 - 1, 
    //     -(((f32)Input::mouse_pos.y / window_size.y) * 2 -1) } ;


    glClear(GL_COLOR_BUFFER_BIT);

    glBindBuffer(GL_ARRAY_BUFFER, triangle_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_vertices), triangle_vertices.buffer, GL_STATIC_DRAW);
    glUseProgram(Assets::shaders[0]);
    u32 u_mpv_m_loc = glGetUniformLocation(Assets::shaders[0], "u_mvp_m");
    // glUniformMatrix4fv(u_mpv_m_loc, 1, GL_FALSE, )
    glDrawElements(GL_TRIANGLES, cap(&triangle_indexes)*3, GL_UNSIGNED_INT, null);
    
}
