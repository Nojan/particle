#version 100
precision highp float;
attribute vec4 vertexPosition_modelspace;
attribute vec4 vertexColor;

varying vec4 color;

uniform mat4 view;
uniform mat4 projection;
uniform vec2 screenSize;
uniform float spriteSize;

void main()
{
    vec4 eyePos = view * vertexPosition_modelspace;
    vec4 projVoxel = projection * vec4(spriteSize,spriteSize,eyePos.z,eyePos.w);
    vec2 projSize = screenSize * projVoxel.xy / projVoxel.w;
    gl_PointSize = 0.25 * (projSize.x+projSize.y);
    gl_Position = projection * eyePos;
    color = vertexColor;
}

