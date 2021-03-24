
#shader vertex
#version 440 core


layout(location = 0) in vec4 position;
layout(location = 1) in vec2 uv;

out vec2 itpl_uv;
out float itpl_z;


uniform mat4 u_tr_m;


void main() {
    itpl_uv = uv;
    gl_Position = u_tr_m * position;
    // gl_Position.xy /= abs(gl_Position.z);
    gl_Position.w = abs(gl_Position.z);
    itpl_z = gl_Position.z;
}




#shader fragment
#version 440 core


layout(location = 0) out vec4 color;
in vec2 itpl_uv;
in float itpl_z;
uniform sampler2D u_texture;
uniform vec4 u_tex_color;

void main() {
    color = texture(u_texture, itpl_uv) * u_tex_color ;
    color.rgb /= 10 * (itpl_z + 1.0);
}
