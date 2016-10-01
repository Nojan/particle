#version 100
attribute vec3 Position;
attribute vec3 Normal;
attribute vec2 TexCoord0;
attribute vec4 Index;
attribute vec4 Weigth;

varying vec2 UV;
varying vec3 vertexNormal_eyespace;
varying vec3 lightDirection;
varying vec3 eye;

uniform mat4 mvp;
uniform mat4 mv;
uniform mat3 viewNormal;
uniform vec4 lightPosition;
const int MAX_BONES = 32;
uniform mat4 bones[MAX_BONES];

void main() {
    mat4 BoneTransform = bones[int(Index[0])] * Weigth[0];
    BoneTransform += bones[int(Index[1])] * Weigth[1];
    BoneTransform += bones[int(Index[2])] * Weigth[2];
    BoneTransform += bones[int(Index[3])] * Weigth[3];

    UV = TexCoord0;
    vec4 vertex_position = BoneTransform * vec4(Position, 1);
    vec4 position_viewspace = mv * vertex_position;
    vertexNormal_eyespace = viewNormal * Normal;
    vec4 lightPosition_eyespace = mv * lightPosition;
    lightDirection = viewNormal * vec3(lightPosition) + vec3(0.01) * normalize(vec3(lightPosition_eyespace - position_viewspace));
    eye = -vec3(position_viewspace);
    gl_Position = mvp * vertex_position;
}
