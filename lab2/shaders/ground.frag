#version 330 core

in vec2 fragmentUV;
in vec3 fragmentNormal;
in vec3 fragmentPosition;
in vec4 fragPosLightSpace;

uniform sampler2D textureSampler;
uniform sampler2D shadowMap;
uniform vec3 lightPosition;
uniform vec3 lightIntensity;
uniform vec3 lightColor;
uniform mat4 lightSpaceMatrix;

out vec4 color;
uniform int ground;


void main() {
    //just shw the textor without taking into acount ilumination
    color = texture(textureSampler, fragmentUV);


}