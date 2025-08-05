/* Equirectangular To CubeMap */

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
uniform sampler2D etHDRTexture;

void main() {
    vec3 nPos = normalize(vPos);
    float theta = asin(nPos.y);
    float phi = atan(-nPos.x, nPos.z);

    vec2 uv;
    uv.x = 0.5 + phi / (2 * PI);
    uv.y = 0.5 + theta / PI;

    vec3 result = texture(etHDRTexture, uv).rgb;
    FragColor = vec4(result, 1.0);
}