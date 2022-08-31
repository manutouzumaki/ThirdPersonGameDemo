#version 330 core

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

in vec3 position;
in vec3 normal;
in vec2 texCoord;

out vec3 uv;

void main() {
    uv = position;
    gl_Position = projection * mat4(mat3(view)) * model * vec4(position, 1.0);
}
