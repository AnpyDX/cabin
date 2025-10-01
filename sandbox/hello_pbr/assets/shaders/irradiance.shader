#![version("430 core")]

#![vertex]
layout (location = 0) in vec3 aPos;

uniform mat4 view;
uniform mat4 projection;

out vec3 vPos;

void main() {
    gl_Position = projection * view * vec4(aPos, 1.0);
    vPos = aPos;
}

#![fragment]
#![use("consts.utils")]
out vec4 FragColor;

in vec3 vPos;

uniform samplerCube envCubeMap;

const float sampleStep = 0.0125;

void main() {
    vec3 N = normalize(vPos);
    vec3 T = normalize(cross(vec3(0.0, 1.0, 0.0), N));
    vec3 B = normalize(cross(N, T));

    float sampleScale = 0.0;
    vec3 irradiance = vec3(0.0);
    for (float phi = 0.0; phi < 2.0 * PI; phi += sampleStep) {
        for (float theta = 0.0; theta < 0.5 * PI; theta += sampleStep) {
            vec3 sampleDirection = sin(theta) * N + 
                                   cos(theta) * cos(phi) * T +
                                   cos(theta) * sin(phi) * B;
            irradiance += texture(envCubeMap, sampleDirection).rgb * cos(theta) * sin(theta);
            sampleScale += 1.0;
        }
    }

    vec3 result = PI * irradiance / sampleScale;
    FragColor = vec4(result, 1.0);
}