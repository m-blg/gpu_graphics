#version 330 core


layout(location = 0) out vec4 color;
in vec4 itpl_color;


void main() {
    color = itpl_color;
}


