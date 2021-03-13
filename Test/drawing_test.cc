
#include "../SDL_main_opengl.cc"

#include "cp_lib/basic.cc"
#include "cp_lib/array.cc"
#include "cp_lib/vector.cc"
#include "cp_lib/memory.cc"
#include "cp_lib/io.cc"
#include "cpu_graphics/draw.cc"
#include <stdlib.h>
#include <unistd.h>

struct Triangle_Vertex {
    vec2f pos;
    vec4f color;
};

sbuff<Triangle_Vertex, 4> triangle_vertices = {{
    {{1.0f, 0.5f}, {1, 1, 0, 0} }, 
    {{-1.0f, 0}, {1, 0, 0, 1} },
    {{0, -0.5f}, {1, 0, 1, 0}},
    {{1, -0.5f}, {1, 0, 1, 0}},
}};

sbuff<u32[3], 2> triangle_indexes = {{
    {0, 1, 2},
    {0, 2, 3}
}};

float triangle_color[3][4] = {{1, 1, 0, 0}, {1, 0, 0, 1}, {1, 0, 1, 0}};
dbuff2f triangle_color_buffer = {(f32*)triangle_color, 3, 4};

u32 triangle_buffer_id;



u32 CompileShaderFromFile(u32 shader_type, const char* file_name) {
    u32 shader_id = glCreateShader(shader_type);
    dstr shader_src;
    read_whole(&shader_src, file_name);
    i32 src_size = (i32)shader_src.cap;
    glShaderSource(shader_id, 1, &shader_src.buffer, &src_size);
    glCompileShader(shader_id);

    i32 cpl_res;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &cpl_res);
    if (cpl_res == GL_FALSE) {
        GLsizei log_length = 0;
        GLchar message[1024];
        glGetShaderInfoLog(shader_id, 1024, &log_length, message);
        printf("Shader Compilation Failed (%s)\n", 
            (shader_type == GL_VERTEX_SHADER ? "vertex" : "fragment"));
        puts(message);
        glDeleteShader(shader_id);
        return 0;
    }

    shader_src.shut();
    return shader_id;
}


void gl_debug_callback(GLenum source,
            GLenum type, GLuint id,
            GLenum severity, GLsizei length,
            const GLchar *message, const void *userParam) 
{
    puts(message);
}




void game_init() {
    Input::input_init();
    window_size = {1280, 720};

    puts((char*)glGetString(GL_VERSION));

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(gl_debug_callback, null);

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


    // make shaders

    u32 vshader_id = CompileShaderFromFile(GL_VERTEX_SHADER, "Test/vshader.glsl");
    u32 fshader_id = CompileShaderFromFile(GL_FRAGMENT_SHADER, "Test/fshader.glsl");

    u32 pshader_id = glCreateProgram();
    glAttachShader(pshader_id, vshader_id);
    glAttachShader(pshader_id, fshader_id);
    glLinkProgram(pshader_id);
    glValidateProgram(pshader_id);

    glDeleteShader(vshader_id);
    glDeleteShader(fshader_id);

    glUseProgram(pshader_id);
    //


    glClearColor(0, 0.2, 0.2, 0);

}

void game_shut() {
    Input::input_shut();
}


void game_update() {


    if (get_bit(Input::keys_down, 'q')) {
        is_running = false;
    }

    triangle_vertices[0].pos = { ((f32)Input::mouse_pos.x / window_size.x) * 2 - 1, 
        -(((f32)Input::mouse_pos.y / window_size.y) * 2 -1) } ;


    glClear(GL_COLOR_BUFFER_BIT);

    glBindBuffer(GL_ARRAY_BUFFER, triangle_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_vertices), triangle_vertices.buffer, GL_DYNAMIC_DRAW);
    glDrawElements(GL_TRIANGLES, cap(&triangle_indexes)*3, GL_UNSIGNED_INT, null);
    
}
