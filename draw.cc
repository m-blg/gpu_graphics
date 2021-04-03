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

struct {
    u32 vao;
    u32 vbo;
    u32 ibo;
    u32 shader;
    u32 texures[10];
} gl_cache;

void bind_vao(u32 vao) {
    if (vao != gl_cache.vao) {
        glBindVertexArray(vao);
        gl_cache.vao = vao;
    }
}


void bind_vbo(u32 vbo) {
    if (vbo != gl_cache.vbo) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        gl_cache.vbo = vbo;
    }
}


void bind_ibo(u32 ibo) {
    if (ibo != gl_cache.ibo) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        gl_cache.ibo = ibo;
    }
}


void bind_shader(u32 shader) {
    if (shader != gl_cache.shader) {
        glUseProgram(shader);
        gl_cache.shader = shader;
    }
}

void bind_texture(u32 texture, i32 slot) {
    if (texture != gl_cache.texures[slot]) {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, texture);
        gl_cache.texures[slot] = texture;
    }
}

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

struct Material_Sprite2D {
    u32 shader_name;
    u32 texture_name;
    vec4f color;
};

struct Transform {
    vec3f position;
    quat rotation;
    vec3f scale;

    void init();
};

void Transform::init() {
    position = {};
    rotation = { 1, 0, 0, 0 }; 
    scale = { 1, 1, 1 };
}



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

template <typename t_Material>
struct Render_Object {
    Transform *transform;
    Mesh *mesh;
    t_Material *material;
    u32 vbo;
    u32 ibo;
};

template <typename t_Render_Object>
void render(t_Render_Object *self) {
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


vec2f screen_to_view_space(vec2f p, vec2f window_size, vec2f pixels_per_unit) {
    vec2f centered = p - window_size / 2.0f;
    return {centered.x / pixels_per_unit.x, -centered.y / pixels_per_unit.y};
}

vec2f screen_to_world_space(vec2f p, Transform camera_transform, vec2f window_size, vec2f pixels_per_unit) {
    vec2f view = screen_to_view_space(p, window_size, pixels_per_unit);
    return view + vec2f(camera_transform.position.x, camera_transform.position.y);
}

vec2f closest_point_line(vec2f p1, vec2f p2, vec2f p) {
    vec2f u = p2 - p1;
    f32 t = - dot(u, (p1 - p)) / dot(u, u);
    return p1 + t * u;
}

vec2f closest_point_segment(vec2f p1, vec2f p2, vec2f p) {
    vec2f u = p2 - p1;
    f32 t = - dot(u, (p1 - p)) / dot(u, u);
    t = max(0.0f, min(1.0f, t));
    return p1 + t * u;
}


template <typename T>
struct Rect {
    vec2<T> lb;
    vec2<T> rt;
};

template <typename T1, typename T2>
bool is_contained(Rect<T1> rect, vec2<T2> p) {
    return (rect.lb.x <= p.x && p.x <= rect.rt.x && rect.lb.y <= p.y && p.y <= rect.rt.y);
}

template <typename T>
bool do_intersect(Rect<T> rect1, Rect<T> rect2) {
    return (
        rect1.lb.x < rect2.rt.x &&
        rect1.lb.y < rect2.rt.y &&
        rect1.rt.x > rect2.lb.x &&
        rect1.rt.y > rect2.lb.y
    );
}

template <typename T>
vec2<T> centerof(Rect<T> rect) {
    return (rect.lb + rect.rt) / (T)2;
}

