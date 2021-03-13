#pragma once
#include "SDL_main_opengl.cc"

#include "cp_lib/io.cc"


u32 compile_sub_shader(u32 shader_type, dbuff<char> shader_src) {
    u32 shader_id = glCreateShader(shader_type);
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

u32 compile_shader_from_file(const char* file_name) {
    dstr shader_text;
    read_whole(&shader_text, file_name);

    char vsh_token[] = "#shader vertex";
    char* vsh_token_p = strstr(begin(&shader_text), vsh_token);
    char* vsh_bp = vsh_token_p + sizeof(vsh_token);

    char fsh_token[] = "#shader fragment";
    char* fsh_token_p = strstr(vsh_bp, fsh_token);
    char* fsh_bp = fsh_token_p + sizeof(fsh_token);

    u32 vshader_id = compile_sub_shader(GL_VERTEX_SHADER, {vsh_bp, u32(fsh_token_p - vsh_bp) });
    u32 fshader_id = compile_sub_shader(GL_FRAGMENT_SHADER, {fsh_bp, u32(end(&shader_text) - fsh_bp) });

    u32 pshader_id = glCreateProgram();
    glAttachShader(pshader_id, vshader_id);
    glAttachShader(pshader_id, fshader_id);
    glLinkProgram(pshader_id);
    glValidateProgram(pshader_id);

    glDeleteShader(vshader_id);
    glDeleteShader(fshader_id);
}

void load_assets() {

}
