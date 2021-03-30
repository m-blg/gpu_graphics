

#define GUI_ENABLED 1
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
    { -0.5f, -0.5f, 0 },
	{ 0.5f, -0.5f, 0 },
	{ 0.5f, 0.5f, 0 },
	{ -0.5f, 0.5f, 0 }
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
    Collider collider;
    vec2f velocity;

    Material_Sprite2D material;
};

darr<Physics_Object> quads;

Physics_Object* selected_object = null;

Transform camera_transform = {
    { 0, 0, 0 },
    { 1, 0, 0, 0},
    { 0.5, 0.5, 0.5 }
};

Transform* main_camera;

void save_physics_objects(const char* file_name) {
    FILE* file = fopen(file_name, "wb");
    fwrite(&quads.len, sizeof(u32), 1, file);
    fwrite(quads.buffer, sizeof(Physics_Object), quads.len, file);
    fclose(file);
}

void load_physics_objects(const char* file_name) {
    FILE* file = fopen(file_name, "rb");
    if (file == null) 
        return;

    u32 len;
    fread(&len, sizeof(u32), 1, file);
    quads.init(len);
    quads.len = len;
    fread(quads.buffer, sizeof(Physics_Object), len, file);
    fclose(file);
}

void render_quads() {
    bind_shader(Assets::shaders[0].id);

    bind_vao(quad_vao);
    bind_ibo(quad_ibo);

    mat4f vp_m = proj_xy_orth_matrix(window_size, {100, 100}, {-1, 30}) * view_matrix(&camera_transform);

    for (auto it = begin(&quads); it != end(&quads); it++) {
        i32 texture_slot = 0;
        bind_texture(Assets::textures[it->material.texture_name].id, texture_slot);

        set_uniform(&Assets::shaders[0], 1, texture_slot);
        vec4f color = it->material.color;
        set_uniform(&Assets::shaders[0], 2, color);

        mat4f mvp_m = vp_m * model_matrix(&it->transform);
        set_uniform(&Assets::shaders[0], 0, mvp_m);

        glDrawElements(GL_TRIANGLES, cap(&quad_mesh.index_buffer) * 3, GL_UNSIGNED_INT, null);
    }
}

void render_colliders() {
    glUseProgram(Assets::shaders[0].id);

    glBindVertexArray(quad_vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_ibo);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, Assets::textures[1].id);

    i32 texture_slot_index = 1;
    set_uniform(&Assets::shaders[0], 1, texture_slot_index);
    vec4f color = { 1, 1, 1, 0.5f };
    set_uniform(&Assets::shaders[0], 2, color);


    mat4f vp_m = proj_xy_orth_matrix(window_size, {100, 100}, {-1, 30}) * view_matrix(&camera_transform);

    for (auto it = begin(&quads); it != end(&quads); it++) {
        Box_Collider2D& bc = it->collider.box_collider2d;
        vec2f collider_size = bc.rt - bc.lb;
        vec2f collider_center = (bc.rt + bc.lb) / 2.0f;
        Transform t = it->transform;
        t.position += vec3f(collider_center.x, collider_center.y, 0);
        t.scale = { t.scale.x * collider_size.x, t.scale.y * collider_size.y, t.scale.z };
        mat4f mvp_m = vp_m * model_matrix(&t);

        set_uniform(&Assets::shaders[0], 0, mvp_m);

        glDrawElements(GL_TRIANGLES, cap(&quad_mesh.index_buffer) * 3, GL_UNSIGNED_INT, null);
    }
}

Collider world_space_collider(Physics_Object *po) {
    Collider c;
    c.type = po->collider.type;

    mat4f model_m1 = model_matrix(&po->transform);
    switch (po->collider.type) {
        case Collider_Type::Box_Collider2D: 
        {
            Box_Collider2D& po_col = po->collider.box_collider2d;

            c.box_collider2d = { (vec2f)(model_m1 * vec4f(po_col.lb, 0, 1)), 
                (vec2f)(model_m1 * vec4f(po_col.rt, 0, 1)) }; 
        } break;
        case Collider_Type::Sphere_Collider2D:
        {
            Sphere_Collider2D& po_col = po->collider.sphere_collider2d;

            c.sphere_collider2d = { (vec2f)(model_m1 * vec4f(po_col.origin, 0, 1)), po_col.radius * max(po->transform.scale.x, po->transform.scale.y) };
            
        } break;
    }

    return c;
}

void physics_update() {
    for (auto it = begin(&quads); it != end(&quads); it++) {
        it->transform.position += vec3f(it->velocity, 0) * GTime::dt;
    }
    for (auto it = begin(&quads); it != end(&quads); it++) {
        //mat4f model_m1 = model_matrix(&it->transform);
        //Box_Collider2D c1 = { (vec2f)(model_m1 * vec4f(it->collider.lb, 0, 1)), (vec2f)(model_m1 * vec4f(it->collider.rt, 0, 1)) }; 
        auto c1 = world_space_collider(it);
        for (auto it2 = it+1; it2 != end(&quads); it2++) {
            auto c2 = world_space_collider(it2);
            if (do_collide(&c1, &c2)) {
                it->velocity = -1.0f * it->velocity;
                it2->velocity = -1.0f *it2->velocity;
            }
        }
    }
}

Physics_Object* is_over(vec2f p) {
    f32 depth = INT_MIN;
    Physics_Object* po = null;
    for (auto it = begin(&quads); it != end(&quads); it++) {
        Collider c = world_space_collider(it);
        // vec2f local_p = p - vec2f(it->transform.position.x, it->transform.position.y);
        if (is_contained(&c, p) && it->transform.position.z > depth) {
            po = it;
            depth = it->transform.position.z;
        }
    }
    return po;
}

void game_init() {
    Input::input_init();

    Assets::load_shaders<1>({"Test/sprite.glsl"});
    Assets::shaders[0].init(3);
    add_uniform(&Assets::shaders[0], "u_mpv_mat", Type::mat4f);
    add_uniform(&Assets::shaders[0], "u_texture", Type::i32);
    add_uniform(&Assets::shaders[0], "u_color", Type::vec4f);

    Assets::load_textures<2>({"Test/Assets/TestTexture.png", "Test/Assets/BoxCollider2D.png"});


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
    

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Assets::textures[0].id);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

    //glEnable(GL_DEPTH_TEST);
    //glDepthFunc(GL_LESS);

    glClearColor(0, 0.2, 0.2, 1);

    main_camera = &camera_transform;

    load_physics_objects("Test/Assets/save.bin");

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

    ImGuiIO& gui_io = ImGui::GetIO();
    if (!gui_io.WantCaptureMouse && Input::is_mouse_button_down(0)) {

        vec2f cursor_world_pos = screen_to_world_space(Input::mouse_position, *main_camera, window_size, {100, 100});
        selected_object = is_over(cursor_world_pos);
        if (selected_object != null) return;

        Transform t; t.init();
        t.position = {cursor_world_pos.x, cursor_world_pos.y, 0};
        Collider collider = {.type = Collider_Type::Box_Collider2D};
        collider.box_collider2d = { {-0.25f, -0.25f}, {0.25f, 0.25f} };
        Material_Sprite2D material = { 0, 0, {1, 1, 1, 1}};
        dpush(&quads, {t, collider, {}, material});
    }

    render_quads();
    render_colliders();

    physics_update();

    // cube_transform.position += vec3f(0.5, 0.5, -1);
    // to_mat4(&tr_m, &cube_transform);
    // glUniformMatrix4fv(mvp_mat_loc, 1, GL_TRUE, (f32*)&tr_m);
    // glDrawElements(GL_TRIANGLES, cap(&quad_mesh.index_buffer) * 3, GL_UNSIGNED_INT, null);
    // cube_transform.position -= vec3f(0.5, 0.5, -1);


}

#if GUI_ENABLED
#include "../import/imgui/imgui_demo.cpp"
void draw_gui() {
    ImGui::Begin("Texture color");
    ImGui::ShowDemoWindow();
    // ImGui::ColorPicker4("Color", begin(&tex_color.buffer));
    ImGui::Text("dt: %f", 1 / GTime::dt);

    ImGui::Separator();

    ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | 
        ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    
    if (ImGui::TreeNode("Physics Objects")) {
        for (u32 i = 0; i < len(&quads); i++) {
            ImGuiTreeNodeFlags node_flags;
            if (&quads[i] == selected_object) {
                node_flags |= ImGuiTreeNodeFlags_Selected;
            }
            ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, "Physcics Object %d", i);
            if (ImGui::IsItemClicked()) {
                selected_object = &quads[i];
            }
            // ImGui::TreePop();
        }
        ImGui::TreePop();
    }

    
    if (selected_object != null) {
        if (ImGui::CollapsingHeader("Transform")) {
            ImGui::SliderFloat3("Position", (f32*)(&selected_object->transform.position), -10, 10);
            ImGui::SliderFloat4("Rotation", (f32*)(&selected_object->transform.rotation), -10, 10);
            ImGui::SliderFloat3("Scale", (f32*)(&selected_object->transform.scale), -10, 10);
        }
        if (ImGui::CollapsingHeader("Collider")) {
            if (selected_object->collider.type == Collider_Type::Box_Collider2D) {
                Box_Collider2D& collider = selected_object->collider.box_collider2d;
                ImGui::Text("Box Collider2D");
                ImGui::SliderFloat2("lb", (f32*)(&collider.lb), -10, 10);
                ImGui::SliderFloat2("rt", (f32*)(&collider.rt), -10, 10);
            } else if (selected_object->collider.type == Collider_Type::Sphere_Collider2D) {
                Sphere_Collider2D& collider = selected_object->collider.sphere_collider2d;
                ImGui::Text("Sphere Collider2D");
                ImGui::SliderFloat2("origin", (f32*)(&collider.origin), -10, 10);
                ImGui::SliderFloat("radius", (f32*)(&collider.radius), -10, 10);
            }

        }
        ImGui::Separator();
        ImGui::SliderFloat2("Velocity", (f32*)(&selected_object->velocity), -10, 10);
        if (ImGui::Button("Delete")) {
            remove(&quads, selected_object);
            selected_object = null;
        }
    }


    if (ImGui::Button("Save")) {
        save_physics_objects("Test/Assets/save.bin");
    }
    ImGui::End();
}
#else

void draw_gui() {}

#endif

