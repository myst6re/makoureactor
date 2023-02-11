#version 410 core

in vec4 a_position;
in vec4 a_color;
in vec2 a_texcoord;

out vec4 v_color;
out vec2 v_texcoord;

uniform mat4 modelMatrix;
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform float pointSize;

void main()
{
    mat4 mat = projectionMatrix * viewMatrix * modelMatrix;

    gl_Position = mat * a_position;
    gl_PointSize = pointSize;
    v_color = a_color;
    v_texcoord = a_texcoord;
}
