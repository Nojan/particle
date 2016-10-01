#version 100
precision highp float;
varying vec2 UV;
varying vec3 vertexNormal_eyespace;
varying vec3 lightDirection;
varying vec3 eye;

uniform sampler2D textureSampler;
uniform vec4 lightDiffuse;
uniform vec4 lightSpecular;

vec4 blinn_phong(const in vec3 fragNormal, const in vec3 lightDir,  const in vec3 eye) {
    vec4 diffuse = vec4(0.5);
    vec4 ambient = lightDiffuse;
    vec4 specular = lightSpecular;
    float shininess = 2.0;

    float intensity = max(dot(fragNormal, lightDir), 0.0);
    vec4 spec = vec4(0.0);
    if (0.0 < intensity) {
        vec3 h = normalize(lightDir + eye);
        float intSpec = max(dot(h, fragNormal), 0.0);
        spec = specular * pow(intSpec, shininess);
    }
    return max(intensity * diffuse + spec, ambient);
}

void main() {
    vec3 vertexNormal_eyespace_normalized = normalize(vertexNormal_eyespace);
    vec3 lightDirectionNormalized = normalize(lightDirection);
    vec3 eye_normalized = normalize(eye);
    vec4 color = blinn_phong(vertexNormal_eyespace_normalized, lightDirectionNormalized, eye_normalized);
    color = color * texture2D( textureSampler, fract(UV) );
    gl_FragColor = color;
}
