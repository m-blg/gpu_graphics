

#define GUI_ENABLED 1
#include "../SDL_main.cc"


#include "../loadings.cc"
#include "../draw.cc"

#include "cp_lib/basic.cc"
#include "cp_lib/array.cc"
#include "cp_lib/vector.cc"
#include "cp_lib/memory.cc"
#include "cp_lib/io.cc"


using namespace cp;

sbuff<vec3f, 8> cube_vrt_positions = {{
    { -1, -1, -1 },
	{ 1, -1, -1 },
	{ 1, 1, -1 },
	{ -1, 1, -1 },
    { -1, 1, 1 },
	{ 1, 1, 1 },
	{ 1, -1, 1 },
	{ -1, -1, 1 }
}};

sbuff<vec2f, 8> cube_uvs = {{
    {0, 0},
    {1, 0},
    {1, 1},
    {0, 1},
    {0, 0},
    {1, 0},
    {1, 1},
    {0, 1}
}};

sbuff<u32[3], 10> cube_triangles = {{
    //{0, 2, 1}, //face front
    //{0, 3, 2},
    {2, 3, 4}, //face top
    {2, 4, 5},
    {1, 2, 5}, //face right
    {1, 5, 6},
    {0, 7, 4}, //face left
    {0, 4, 3},
    {5, 4, 7}, //face back
    {5, 7, 6},
    {0, 6, 7}, //face bottom
    {0, 1, 6}
}};

// sbuff<u32[2], 12> cube_edges = {{
//     {0, 1}, {0, 3}, {1, 2}, {2, 3},
//     {4, 5}, {4, 7}, {6, 5}, {6, 7},
//     {0, 7}, {1, 6}, {2, 5}, {3, 4},
// }};

Mesh cube_mesh = {
    { begin(&cube_vrt_positions), cap(&cube_vrt_positions) }, 
    { begin(&cube_triangles), cap(&cube_triangles) }
};

u32 cube_vao_id;
u32 cube_vbo_id;
u32 cube_ibo_id;
u32 tr_m_loc;

Transform cube_transform = {
    { 0, 0, 5 },
    { 1, 0, 0, 0},
    { 0.5, 0.5, 0.5 }
};

bool is_ortho = true;


dbuff2f z_buffer;

static quat rot_x;
static quat rot_y;
static quat rot_z;

dbuff2u test_texture;
u32 u_texture_loc;

vec4f tex_color = {1, 1, 1, 1};

Transform camera_transform = {
    { 0, 0, 0 },
    { 1, 0, 0, 0},
    { 0.5, 0.5, 0.5 }
};


void game_init() {
    Input::input_init();
    Assets::load_shaders<1>({"Test/test_texture_shader.glsl"});
    Assets::load_textures<1>({"Test/Assets/TestTexture.png"});
    window_size = {1280, 720};


    glGenVertexArrays(1, &cube_vao_id);
    glBindVertexArray(cube_vao_id);

    // make buffer for triangle
    glGenBuffers(1, &cube_vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo_id);

    // allocates vram
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vrt_positions) + sizeof(cube_uvs), null, GL_STATIC_DRAW);
    
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(cube_vrt_positions), begin(&cube_vrt_positions));
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(cube_vrt_positions), sizeof(cube_uvs), begin(&cube_uvs));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)sizeof(cube_vrt_positions));

    glGenBuffers(1, &cube_ibo_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_ibo_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_triangles), begin(&cube_triangles), GL_STATIC_DRAW);


    glUseProgram(Assets::shaders[0].id);
    tr_m_loc = glGetUniformLocation(Assets::shaders[0].id, "u_tr_m");
    u_texture_loc = glGetUniformLocation(Assets::shaders[0].id, "u_texture");

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Assets::textures[0].id);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glClearColor(0, 0.2, 0.2, 1);

    rot_x.init(normalized(vec3f{1, 0, 0}), M_PI/10);
    rot_y.init(normalized(vec3f{0, 1, 0}), M_PI/10);
    rot_z.init(normalized(vec3f{0, 0, 1}), M_PI/10);

}

void game_shut() {
    Input::input_shut();
}

void tex_color_gui(vec4f *tex_color) {

}


void game_update() {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    if (get_bit(Input::keys_down, 'q')) {
        is_running = false;
    }
    if (get_bit(Input::keys_down, 'o')) {
        is_ortho = !is_ortho;
    }

    if (get_bit(Input::keys_hold, 'w')) {
        camera_transform.position += vec3f(0, 0, 0.1);
    }
    if (get_bit(Input::keys_hold, 's')) {
        camera_transform.position += vec3f(0, 0, -0.1);
    }
    if (get_bit(Input::keys_hold, 'a')) {
        camera_transform.position += vec3f(-0.1, 0, 0);
    }
    if (get_bit(Input::keys_hold, 'd')) {
        camera_transform.position += vec3f(0.1, 0, 0);
    }
    if (get_bit(Input::keys_hold, 't')) {
        cube_transform.rotation = rot_x * cube_transform.rotation;
    }    
    if (get_bit(Input::keys_hold, 'g')) {
        cube_transform.rotation = inverse(rot_x) * cube_transform.rotation;
    }
    if (get_bit(Input::keys_hold, 'f')) {
        cube_transform.rotation = rot_y * cube_transform.rotation;
    }
    if (get_bit(Input::keys_hold, 'h')) {
        cube_transform.rotation = inverse(rot_y) * cube_transform.rotation;
    }
    if (get_bit(Input::keys_hold, 'r')) {
        cube_transform.rotation = rot_z * cube_transform.rotation;
    }
    if (get_bit(Input::keys_hold, 'y')) {
        cube_transform.rotation = inverse(rot_z) * cube_transform.rotation;
    }

    glUseProgram(Assets::shaders[0].id);
    mat4f mvp_m;
    mvp_m = proj_xy_orth_matrix(window_size, {100, 100}, {-1, 30}) * view_matrix(&camera_transform) *  model_matrix(&cube_transform);
    glUniformMatrix4fv(tr_m_loc, 1, GL_TRUE, (f32*)&mvp_m);
    glUniform1i(u_texture_loc, 0);
    u32 u_tex_color_loc = glGetUniformLocation(Assets::shaders[0].id, "u_tex_color");

    glUniform4f(u_tex_color_loc, tex_color.x, tex_color.y, tex_color.z, tex_color.w);

    glBindVertexArray(cube_vao_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_ibo_id);
    glBindTexture(GL_TEXTURE_2D, Assets::textures[0].id);
    glDrawElements(GL_TRIANGLES, cap(&cube_triangles) * 3, GL_UNSIGNED_INT, null);

    // cube_transform.position += vec3f(0.5, 0.5, -1);
    // to_mat4(&tr_m, &cube_transform);
    // glUniformMatrix4fv(tr_m_loc, 1, GL_TRUE, (f32*)&tr_m);
    // glDrawElements(GL_TRIANGLES, cap(&cube_triangles) * 3, GL_UNSIGNED_INT, null);
    // cube_transform.position -= vec3f(0.5, 0.5, -1);


}

#if GUI_ENABLED

void draw_gui() {
    ImGui::Begin("Texture color");
    ImGui::ColorPicker4("Color", begin(&tex_color.buffer));
    ImGui::SliderFloat3("cube pos", (f32*)&cube_transform.position, -10, 10);
    ImGui::End();
}
#else

void draw_gui() {}

#endif

