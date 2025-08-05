#![version("430 core")]

#![vertex]
layout (location = 0) in vec3 aPos;

uniform mat4 view;
uniform mat4 projection;

out vec3 vPos;

void main() {
    gl_Position = (projection * view * vec4(aPos, 1.0)).xyww;
    vPos = aPos;
}

#![fragment]
out vec4 FragColor;

in vec3 vPos;
uniform samplerCube envCubeMap;

void main() {
    vec3 result = texture(envCubeMap, vPos).rgb;
    result = result / (result + vec3(1.0));
    result = pow(result, vec3(1.0 / 2.2));
    
    FragColor = vec4(result, 1.0);
}