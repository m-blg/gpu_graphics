#pragma once

#include "SDL_main_opengl.cc"

#include "cp_lib/basic.cc"
#include "cp_lib/buffer.cc"
#include "cp_lib/vector.cc"
#include "cp_lib/math.cc"
#include <math.h>

using namespace cp;

struct Mesh {
    desbuff vertex_buffer;
    dbuff<u32[3]> index_buffer;
};

struct Material {
    u32 shader_id;
    // shader data
    // textures
};

struct Render_Object {
    Mesh *mesh;
    Material *material;
    u32 vbo_id;
    u32 ibo_id;
};

void render(Render_Object *self) {
    glUseProgram(self->material->shader_id);
    glBindBuffer(GL_ARRAY_BUFFER, self->vbo_id);
    glBufferData(GL_ARRAY_BUFFER, cap(&self->mesh->vertex_buffer), begin(&self->mesh->vertex_buffer), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, self->mesh->vertex_buffer.stride, 0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, self->mesh->vertex_buffer.stride, (void*)sizeof(vec2f));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);


    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->ibo_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cap(&self->mesh->index_buffer) * sizeof(u32[3]), self->mesh->index_buffer.buffer, GL_DYNAMIC_DRAW);


    glDrawElements(GL_TRIANGLES, cap(&self->mesh->index_buffer) * 3, GL_UNSIGNED_INT, self->mesh->index_buffer.buffer);
}


// cap(&vertices) <= cap(&out_points)
void project_xy_orthogonal(dbuff<vec3f> vertices, dbuff<vec2f> out_points) {
    for (u32 i = 0; i < cap(&vertices); i++) {
        vec3f *vertex = &vertices[i];
        out_points[i] = { vertex->x, vertex->y };
    }
}

vec2f project_xy_orthogonal(vec3f vertex) {
    return { vertex.x, vertex.y };
}


// cap(&vertices) <= cap(&out_points)
void project_xy_perspective(dbuff<vec3f> vertices, dbuff<vec2f> out_points) {
    for (u32 i = 0; i < cap(&vertices); i++) {
        vec3f *vertex = &vertices[i];
        out_points[i] = { vertex->x / vertex->z, vertex->y / vertex->z };
    }
}

vec2f project_xy_perspective(vec3f vertex) {
    return { vertex.x / vertex.z, vertex.y / vertex.z };
}

