#version 100
precision highp float;
varying vec2 UV;

uniform sampler2D textureSampler;
uniform float alpha;

void main() {
    vec4 alpha4 = vec4(1, 1, 1, alpha);
    vec4 color = texture2D( textureSampler, fract(UV) );
    if( color.w < 0.01 )
        discard; 
    gl_FragColor = color * alpha4;
}
