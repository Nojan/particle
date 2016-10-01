#version 100
precision highp float;
varying vec4 color;

uniform sampler2D uTexture;

void main()
{
    vec4 texture = texture2D(uTexture, gl_PointCoord);
    vec4 c = color;
    c.a = texture.r;
    gl_FragColor = c;
}
