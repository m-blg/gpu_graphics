

#define GUI_ENABLED 0
#include "gpu_graphics/SDL_main.cc"

#include "gpu_graphics/loadings.cc"
#include "gpu_graphics/mesh.cc"

#include "cp_lib/basic.cc"
#include "cp_lib/array.cc"
#include "cp_lib/vector.cc"
#include "cp_lib/memory.cc"
#include "cp_lib/io.cc"


using namespace cp;

sbuff<vec3f, 4> quad_vrt_positions = {{
    { -1, -1, 0 },
	{ 1, -1, 0 },
	{ 1, 1, 0 },
	{ -1, 1, 0 }
}};

sbuff<u32[3], 10> quad_triangles = {{
    {0, 1, 2}, //face front
    {0, 2, 3}
}};


sbuff<vec2f, 4> quad_uvs = {{
    {0, 0},
    {1, 0},
    {1, 1},
    {0, 1}
}};

Mesh quad_mesh = {
    { begin(&quad_vrt_positions), cap(&quad_vrt_positions) }, 
    { begin(&quad_triangles), cap(&quad_triangles) }
};

u32 quad_vao;
u32 quad_vbo;
u32 quad_ibo;
u32 mvp_mat_loc;

struct Physics_Object {
    Transform transform;
    Box_Collider2D collider;
};

darr<Physics_Object> quads;

dbuff2u test_texture;
u32 u_texture_loc;

vec4f tex_color = {1, 1, 1, 1};

Transform camera_transform = {
    { 0, 0, 0 },
    { 1, 0, 0, 0},
    { 0.5, 0.5, 0.5 }
};

Transform* main_camera;

void render_quads() {
    glUseProgram(Assets::shaders[0].id);

    glBindVertexArray(quad_vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_ibo);
    

    // glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Assets::textures[0].id);
    // glUniform1i(u_texture_loc, 0);
    i32 texture_slot_index = 0;
    set_uniform(&Assets::shaders[0], 1, &texture_slot_index);
    vec4f color = { 1, 1, 1, 1 };
    set_uniform(&Assets::shaders[0], 2, &color);


    // glUniform4f(u_texture_color_loc, tex_color.r, tex_color.g, tex_color.b, tex_color.a);

    mat4f vp_m = proj_xy_orth_matrix(window_size, {100, 100}, {-1, 30}) * view_matrix(&camera_transform);

    for (auto it = begin(&quads); it != end(&quads); it++) {
        mat4f mvp_m = vp_m * model_matrix(&it->transform);
        // glUniformMatrix4fv(mvp_mat_loc, 1, GL_TRUE, (f32*)&mvp_m);
        set_uniform(&Assets::shaders[0], 0, &mvp_m);

        glDrawElements(GL_TRIANGLES, cap(&quad_mesh.index_buffer) * 3, GL_UNSIGNED_INT, null);
    }
}

void render_box_colliders2D() {
    glUseProgram(Assets::shaders[0].id);

    glBindVertexArray(quad_vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_ibo);

    glBindTexture(GL_TEXTURE_2D, Assets::textures[1].id);
    glUniform1i(u_texture_loc, 0);


    mat4f vp_m = proj_xy_orth_matrix(window_size, {100, 100}, {-1, 30}) * view_matrix(&camera_transform);

    for (auto it = begin(&quads); it != end(&quads); it++) {
        Box_Collider2D& bc = it->collider;
        vec2f collider_size = bc.rt - bc.lb;
        vec2f collider_center = (collider_size) / 2.0f;
        Transform t = it->transform;
        t.position += vec3f(collider_center.x, collider_size.y, 0);
        t.scale = { t.scale.x * collider_size.x, t.scale.y * collider_size.y, t.scale.z };
        mat4f mvp_m = vp_m * model_matrix(&it->transform);

        glUniformMatrix4fv(mvp_mat_loc, 1, GL_TRUE, (f32*)&mvp_m);

        glDrawElements(GL_TRIANGLES, cap(&quad_mesh.index_buffer) * 3, GL_UNSIGNED_INT, null);
    }
}


void game_init() {
    Input::input_init();
    Assets::load_shaders<1>({"Test/sprite.glsl"});
    Assets::load_textures<2>({"Test/Assets/TestTexture.png", "Test/Assets/BoxCollider2D.png"});
    window_size = {1280, 720};


    glGenVertexArrays(1, &quad_vao);
    glBindVertexArray(quad_vao);

    // make buffer for triangle
    glGenBuffers(1, &quad_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);

    // allocates vram
    glBufferData(GL_ARRAY_BUFFER, size(&quad_mesh.vertex_buffer) + sizeof(quad_uvs), null, GL_STATIC_DRAW);
    
    glBufferSubData(GL_ARRAY_BUFFER, 0, size(&quad_mesh.vertex_buffer), begin(&quad_mesh.vertex_buffer));
    glBufferSubData(GL_ARRAY_BUFFER, size(&quad_mesh.vertex_buffer), sizeof(quad_uvs), begin(&quad_uvs));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)size(&quad_mesh.vertex_buffer));

    glGenBuffers(1, &quad_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size(&quad_mesh.index_buffer), begin(&quad_mesh.index_buffer), GL_STATIC_DRAW);


    glUseProgram(Assets::shaders[0].id);
    mvp_mat_loc = glGetUniformLocation(Assets::shaders[0].id, "u_mpv_mat");
    u_texture_loc = glGetUniformLocation(Assets::shaders[0].id, "u_texture");

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Assets::textures[0].id);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

    //glEnable(GL_DEPTH_TEST);
    //glDepthFunc(GL_LESS);

    glClearColor(0, 0.2, 0.2, 1);

    main_camera = &camera_transform;

    quads.init(1);
    quads.len = 1;
    quads[0].transform.init();

}

void game_shut() {
    Input::input_shut();
}


void game_update() {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    if (Input::is_key_down('q')) {
        is_running = false;
    }

    if (Input::is_key_held('w')) {
        camera_transform.position += vec3f(0, 0.1, 0);
    }
    if (Input::is_key_held('s')) {
        camera_transform.position += vec3f(0, -0.1, 0);
    }
    if (Input::is_key_held('a')) {
        camera_transform.position += vec3f(-0.1, 0, 0);
    }
    if (Input::is_key_held('d')) {
        camera_transform.position += vec3f(0.1, 0, 0);
    }

    if (Input::is_mouse_button_down(0)) {
        Transform t; t.init();
        vec2f temp = screen_to_world_space(Input::mouse_position, *main_camera, window_size, {100, 100});
        t.position = {temp.x, temp.y, 0};
        dpush(&quads, {t});
    }

    render_quads();

    // cube_transform.position += vec3f(0.5, 0.5, -1);
    // to_mat4(&tr_m, &cube_transform);
    // glUniformMatrix4fv(mvp_mat_loc, 1, GL_TRUE, (f32*)&tr_m);
    // glDrawElements(GL_TRIANGLES, cap(&quad_mesh.index_buffer) * 3, GL_UNSIGNED_INT, null);
    // cube_transform.position -= vec3f(0.5, 0.5, -1);


}

#if GUI_ENABLED

void draw_gui() {
    ImGui::Begin("Texture color");
    ImGui::ColorPicker4("Color", begin(&tex_color.buffer));
    ImGui::End();
}
#else

void draw_gui() {}

#endif

