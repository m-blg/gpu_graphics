
#shader vertex
#version 440 core


layout(location = 0) in vec4 position;
layout(location = 1) in vec4 vertex_color;
out vec4 itpl_color;


void main() {
    itpl_color = vertex_color;
    gl_Position = position;
}




#shader fragment
#version 440 core


layout(location = 0) out vec4 color;
in vec4 itpl_color;


void main() {
    color = itpl_color;
}
