#version 100

// Input vertex data, different for all executions of this shader.
attribute vec3 Position;
attribute vec3 Normal;
attribute vec2 TexCoord0;

// Output data ; will be interpolated for each fragment.
varying vec2 UV;
varying vec3 vertexNormal_eyespace;
varying vec3 lightDirection;
varying vec3 eye;

// Values that stay constant for the whole mesh.
uniform mat4 mvp;
uniform mat4 mv;
uniform mat3 viewNormal;
uniform vec4 lightPosition;

void main(){
    UV = TexCoord0;
    vec4 position_viewspace = mv * vec4(Position, 1);
    vertexNormal_eyespace = viewNormal * Normal;
    vec4 lightPosition_eyespace = mv * lightPosition;
    lightDirection = viewNormal * vec3(lightPosition) + vec3(0.01) * normalize(vec3(lightPosition_eyespace - position_viewspace));
    eye = -vec3(position_viewspace);
    gl_Position =  mvp * vec4(Position, 1);
}

