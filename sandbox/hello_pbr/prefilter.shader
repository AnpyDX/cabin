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
#![use("specular.utils")]
out vec4 FragColor;

in vec3 vPos;

uniform float envResolution;
uniform samplerCube envCubeMap;
uniform float roughnessFactor;

const uint SAMPLE_NUM = 1024u;

void main() {
    vec3 N = normalize(vPos);
    vec3 R = N;
    vec3 V = R;

    float sampleScale = 0.0;
    vec3 prefilterColor = vec3(0.0);
    for (uint i = 0u; i < SAMPLE_NUM; i++) {
        vec2 Xi = Hammersley(i, SAMPLE_NUM);
        vec3 H = ImportanceSampleGGX(Xi, N, roughnessFactor);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0);
        if (NdotL > 0.0) {
            float NdotH = max(dot(N, H), 0.0);
            float HdotV = max(dot(H, V), 0.0);
            float D   = DistributionGGX(H, N, roughnessFactor);
            float pdf = (D * NdotH / (4.0 * HdotV)) + 0.0001;

            float resolution = envResolution;
            float saTexel  = 4.0 * PI / (6.0 * resolution * resolution);
            float saSample = 1.0 / (float(SAMPLE_NUM) * pdf + 0.0001);

            float mipLevel = roughnessFactor == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);

            prefilterColor += textureLod(envCubeMap, L, mipLevel).rgb * NdotL;
            sampleScale += 1.0;
        }
    }

    prefilterColor = prefilterColor / sampleScale;
    FragColor = vec4(prefilterColor, 1.0);
}