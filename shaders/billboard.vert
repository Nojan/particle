#version 100

attribute vec3 vertexPosition_modelspace;
attribute vec2 textureCoord;

varying vec2 UV;

uniform mat4 mvp;

void main(){
    UV = textureCoord;
    gl_Position =  mvp * vec4(vertexPosition_modelspace, 1);
}

