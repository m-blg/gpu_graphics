#pragma once

#include <GL/glew.h>

#include "cp_lib/basic.cc"
#include "cp_lib/buffer.cc"
#include "cp_lib/vector.cc"
#include "cp_lib/quaternion.cc"
#include "cp_lib/matrix.cc"
#include "cp_lib/math.cc"
#include <math.h>

using namespace cp;

struct Mesh {
    dbuff<vec3f> vertex_buffer;
    dbuff<u32[3]> index_buffer;

    void init(u32 vb_init_cap, u32 ib_init_cap);
    void shut();
};

void Mesh::init(u32 vb_init_cap, u32 ib_init_cap) {
    vertex_buffer.init(vb_init_cap);
    index_buffer.init(ib_init_cap);
}

void Mesh::shut() {
    vertex_buffer.shut();
    index_buffer.shut();
}

template <u32 t_texture_count>
struct Material {
    u32 shader_name;
    // shader data
    // textures
    u32 texture_names[t_texture_count];

};

struct Transform {
    vec3f position;
    quat rotation;
    vec3f scale;
};

mat4f model_matrix(Transform *transform) {
    vec3f& position = transform->position;
    quat& rotation = transform->rotation;
    vec3f& scale = transform->scale;

    mat4f translation_m = {
        1, 0, 0, position.x,
        0, 1, 0, position.y,
        0, 0, 1, position.z,
        0, 0, 0, 1
    };
    mat4f rotation_m = vec_rot_mat4(rotation);
    mat4f scale_m = {
        scale.x, 0, 0, 0,
        0, scale.y, 0, 0,
        0, 0, scale.z, 0,
        0, 0, 0, 1
    };

    return translation_m * rotation_m * scale_m;
}

mat4f view_matrix(Transform *transform) {
    vec3f& position = transform->position;
    quat& rotation = transform->rotation;

    mat4f translation_m = {
        1, 0, 0, -position.x,
        0, 1, 0, -position.y,
        0, 0, 1, -position.z,
        0, 0, 0, 1
    };
    mat4f rotation_m = vec_rot_mat4(inverse(rotation));

    return rotation_m * translation_m;
}

struct Render_Object {
    Transform *transform;
    Mesh *mesh;
    Material *material;
    u32 vbo_id;
    u32 ibo_id;
};

void render(Render_Object *self) {
    glUseProgram(self->material->shader_id);
    glBindBuffer(GL_ARRAY_BUFFER, self->vbo_id);
    glBufferData(GL_ARRAY_BUFFER, cap(&self->mesh->vertex_buffer), begin(&self->mesh->vertex_buffer), GL_DYNAMIC_DRAW);

    // glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, self->mesh->vertex_buffer.stride, 0);
    // glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, self->mesh->vertex_buffer.stride, (void*)sizeof(vec2f));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);


    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->ibo_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cap(&self->mesh->index_buffer) * sizeof(u32[3]), self->mesh->index_buffer.buffer, GL_DYNAMIC_DRAW);


    glDrawElements(GL_TRIANGLES, cap(&self->mesh->index_buffer) * 3, GL_UNSIGNED_INT, self->mesh->index_buffer.buffer);
}




mat4f proj_xy_orth_matrix(vec2f window_size, vec2f pixels_per_unit, vec2f z_bounds) {
    //vec3f v;
    //v = { v.x * pixels_per_unit.x * 2 / window_size.x, 
          //v.y * pixels_per_unit.y * 2 / window_size.y,  
          //v.z * 2 / (z_bounds.y - z_bounds.x) - 1.0f };

    return {
        pixels_per_unit.x * 2 / window_size.x, 0, 0, 0,
        0, pixels_per_unit.y * 2 / window_size.y, 0, 0,
        0, 0, 2 / (z_bounds.y - z_bounds.x),  -z_bounds.x * 2 / (z_bounds.y - z_bounds.x) - 1.0f,
        0, 0, 0, 1
    };

}


mat4f proj_xy_persp_matrix(vec2f window_size, vec2f pixels_per_unit, vec2f z_bounds) {
    //f32 a1 = pixels_per_unit.x * 2 / window_size.x / ();
    //return {
        //a1, 0, 0, 0,
        //0, pixels_per_unit.y * 2 / window_size.y, 0, 0,
        //0, 0, 2 / (z_bounds.y - z_bounds.x),  -z_bounds.x * 2 / (z_bounds.y - z_bounds.x) - 1.0f,
        //0, 0, 0, 1
    //};
}

