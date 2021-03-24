
#shader vertex
#version 440 core


layout(location = 0) in vec4 position;
layout(location = 1) in vec4 uv;
out vec4 itpl_uv;
out vec2 itpl_pos;
uniform mat4 u_tr_m;


void main() {
    itpl_uv = uv;
    itpl_pos = (u_tr_m * position).xy;
    gl_Position = vec4((u_tr_m * position).xy, 0, 1);
}




#shader fragment
#version 440 core


layout(location = 0) out vec4 color;
in vec4 itpl_uv;
in vec2 itpl_pos;

float f(float x) {
    x = 3 * x;
    return x*x*x;
}

void main() {
    float threshold = 0.01;
    abs(itpl_pos.y - f(itpl_pos.x)) < threshold ? color = vec4(1, 1, 1, 1) : vec4(1, 0, 0, 0);
    // color = itpl_uv;
}
