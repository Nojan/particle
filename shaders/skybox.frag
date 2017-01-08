#version 100
precision highp float;
varying vec3 texCoord;

uniform vec2 screenSize;
uniform vec3 lightDirectionWS;
uniform mat4 viewMatrix;
uniform samplerCube cubemapSampler;

//frustum
uniform float left;
uniform float right;
uniform float top;
uniform float bottom;
uniform float near;
uniform float far;

float saturate(float value)
{
    return clamp(value, 0., 1.);
}

vec3 saturate(vec3 value)
{
    return vec3(saturate(value.x), saturate(value.y), saturate(value.z));
}

//float smoothstep(float edge0, float edge1, float x)
//{
//    float t = saturate((x - edge0) / (edge1 - edge0));
//    return t * t * (3.0 - 2.0 * t);
//}

void main (void) 
{
    const vec4 originWS = vec4(0, 0, 0, 1);
    const vec3 upDirectionWS = vec3(0, 1, 0);
    const vec3 cameraDirectionCS = vec3(0, 0, -1);
    vec3 upDirectionCS = normalize(mat3(viewMatrix) * upDirectionWS);
    vec3 worldOriginCS = vec3(viewMatrix * originWS);
    vec3 lightDirectionCS = normalize(mat3(viewMatrix) * lightDirectionWS);
    mat4 inverseViewModelMatrix = viewMatrix;
    vec4 color = textureCube(cubemapSampler, texCoord);
    vec3 fragSN = vec3(gl_FragCoord.xy / screenSize, gl_FragCoord.z);
    vec3 fragNDC = fragSN * vec3(2.) - vec3(1.);
    float zeye = 2.*far*near / (fragNDC.z*(far-near)-(far+near));
    float xeye = -zeye*(fragNDC.x*(right-left)+(right+left))/(2.*near);
    float yeye = -zeye*(fragNDC.y*(top-bottom)+(top+bottom))/(2.*near);
    vec4 fragCS = vec4(xeye, yeye, zeye, 1.);
    vec4 fragPositionWS = fragCS * inverseViewModelMatrix;
    vec3 positionWS = vec3(fragPositionWS) / vec3(fragPositionWS.w);
    //if(0.8 < dot(-normalize(cameraDirectionWS), normalize(positionWS)))
    //{
    //    color.rgb = vec3(0, 1, 0);
    //}
    vec3 fragDirectionCS = normalize(vec3(fragCS) - worldOriginCS);
    {
        float minAngle = cos(radians(85.));
        float maxAngle = cos(radians(15.));
        float angle = dot(fragDirectionCS, upDirectionCS);
        float ratioAngle = smoothstep(minAngle, maxAngle, angle);
        color.rgb = vec3(0, 0, 0.5)*ratioAngle + vec3(0, 0.7, 1.)*(1. - ratioAngle);
    }
    vec3 sun;
    {
        float minAngle = cos(radians(3.));
        float maxAngle = cos(radians(0.5));
        float angle = dot(fragDirectionCS, lightDirectionCS);
        float ratioAngle = smoothstep(minAngle, maxAngle, angle);
        sun = vec3(ratioAngle, ratioAngle, ratioAngle);
    }
    color.rgb = saturate(color.rgb + sun);
    if( abs(fragCS.x) < 0.25 && abs(fragCS.y) < 0.25 )
    {
        color.rgb = vec3(1, 0, 0);
    }
    
    gl_FragColor = color;
}
