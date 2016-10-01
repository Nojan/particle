#version 100

attribute vec3 vertexPositionMS;
attribute vec4 vertexColor;

varying vec4 color;

uniform mat4 mvp;

void main()
{
    gl_Position = mvp * vec4(vertexPositionMS, 1);
    color = vertexColor;
}

