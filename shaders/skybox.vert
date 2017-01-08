#version 100

attribute vec3 vertexPosition;
varying vec3 texCoord;
uniform mat4 MVP;

void main() 
{
    gl_Position = MVP * vec4(vertexPosition, 1.0);
    texCoord = vertexPosition;
}

