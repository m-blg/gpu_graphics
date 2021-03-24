#pragma once
#include <GL/glew.h>

#define STB_IMAGE_IMPLEMENTATION
#include "import/stb_image.h"

#include "cp_lib/io.cc"
#include "cp_lib/array.cc"
#include "cp_lib/vector.cc"

using namespace cp;

namespace Assets {



struct Texture {
    u32 id;
    vec2i size;
    i32 bpp;
    u8* buffer;
};

#define TEXTURE_COUNT 1
sbuff<Texture, TEXTURE_COUNT> textures;

Texture create_texture_from_file(const char* file_name) {
    Texture ret;
    // stbi_set_flip_vertically_on_load(1);
    ret.buffer = stbi_load(file_name, &ret.size.x, &ret.size.y, &ret.bpp, 4);

    glGenTextures(1, &ret.id);
    glBindTexture(GL_TEXTURE_2D, ret.id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, ret.size.x, ret.size.y, 
            0, GL_RGBA, GL_UNSIGNED_BYTE, ret.buffer);
    //glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    return ret;
}

#define SHADER_COUNT 1
sbuff<u32, SHADER_COUNT> shaders;

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

    // shader_src.shut();
    return shader_id;
}

u32 compile_shader_from_file(const char* file_name) {
    dstr shader_text;
    if (!read_whole(&shader_text, file_name)) return 0;

    char vsh_token[] = "#shader vertex";
    char* vsh_token_p = strstr(begin(&shader_text), vsh_token);
    char* vsh_bp = vsh_token_p + sizeof(vsh_token);

    char fsh_token[] = "#shader fragment";
    char* fsh_token_p = strstr(vsh_bp, fsh_token);
    char* fsh_bp = fsh_token_p + sizeof(fsh_token);

    u32 vshader_id = compile_sub_shader(GL_VERTEX_SHADER, {vsh_bp, u32(fsh_token_p - vsh_bp - 1) });
    u32 fshader_id = compile_sub_shader(GL_FRAGMENT_SHADER, {fsh_bp, u32(end(&shader_text) - fsh_bp) });

    u32 pshader_id = glCreateProgram();
    glAttachShader(pshader_id, vshader_id);
    glAttachShader(pshader_id, fshader_id);
    glLinkProgram(pshader_id);

    GLint is_linked;
    glGetProgramiv(pshader_id, GL_LINK_STATUS, &is_linked);
    if (is_linked != GL_TRUE)
    {
        GLsizei log_length = 0;
        GLchar message[1024];
        glGetProgramInfoLog(pshader_id, 1024, &log_length, message);
        puts(message);
    }

    glValidateProgram(pshader_id);

    GLint is_valid;
    glGetProgramiv(pshader_id, GL_VALIDATE_STATUS, &is_valid);
    if (is_valid != GL_TRUE)
    {
        GLsizei log_length = 0;
        GLchar message[1024];
        glGetProgramInfoLog(pshader_id, 1024, &log_length, message);
        puts(message);
    }

    glDeleteShader(vshader_id);
    glDeleteShader(fshader_id);

    shader_text.shut();
    return pshader_id;
}


template <u32 t_shader_count>
void load_shaders(sbuff<const char*, t_shader_count> shader_file_names) {
    for (u32 i = 0; i < t_shader_count; i++) {
        shaders[i] = compile_shader_from_file(shader_file_names[i]);
    }
}


template <u32 t_texture_count>
void load_textures(sbuff<const char*, t_texture_count> texture_file_names) {
    for (u32 i = 0; i < t_texture_count; i++) {
        textures[i] = create_texture_from_file(texture_file_names[i]);
    }
}

void load_assets() {
    const char* shader_file_names[SHADER_COUNT] = {"Test/test_shader.glsl"};
    for (u32 i = 0; i < SHADER_COUNT; i++) {
        shaders[i] = compile_shader_from_file(shader_file_names[i]);
    }
}

}
