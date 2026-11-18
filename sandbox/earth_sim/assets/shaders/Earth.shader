/** Earth Surface Rendering, with PBR.
* - ref: <https://learnopengl-cn.github.io/07%20PBR/01%20Theory/>
*/

#![version("430 core")]

#![vertex]
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;

out vec3 vPosition;
out vec3 vNormal;
out vec2 vTexCoord;

void main() {
    gl_Position = projection * view * model * vec4(aPosition, 1.0);
    vPosition = vec3(model * vec4(aPosition, 1.0));
    vNormal = normalMatrix * aNormal;
    vTexCoord = aTexCoord;
}

#![fragment]
#![use("common/consts.su")]
#![const<float>("SUNLIGHT_INTENSITY")]
#![const<float>("EMISSIVE_INTENSITY")]
out vec4 FragColor;

in vec3 vPosition;
in vec3 vNormal;
in vec2 vTexCoord;

uniform vec3 sunPosition;
uniform vec3 sunLuminance;
uniform vec3 cameraPosition;

uniform sampler2D normalTexture;
uniform sampler2D emissiveTexture;
uniform sampler2D baseColorTexture;
uniform sampler2D metallicRoughnessTexture;

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 getNormalFromTexture(sampler2D tex) {
    vec3 tangentSpaceNormal = texture(tex, vTexCoord).rgb;
    tangentSpaceNormal = 2.0 * tangentSpaceNormal - 1.0;

    vec3 E1 = dFdx(vPosition);
    vec3 E2 = dFdy(vPosition);
    vec2 T1 = dFdx(vTexCoord);
    vec2 T2 = dFdy(vTexCoord);

    vec3 N = normalize(vNormal);
    vec3 T = (T2.x * T1.y - T1.x * T2.y) * (T1.y * E2 - T2.y * E1);
    T = normalize(T - dot(T, N) * N);
    vec3 B = normalize(cross(N, T));

    mat3 TBN = mat3(T, B, N);
    return normalize(TBN * tangentSpaceNormal);
}

void main() {
    vec3 normal = getNormalFromTexture(normalTexture);
    vec3 emissive = texture(emissiveTexture, vTexCoord).rgb;
    vec3 baseColor = texture(baseColorTexture, vTexCoord).rgb;
    float metallic = texture(metallicRoughnessTexture, vTexCoord).b;
    float roughness = texture(metallicRoughnessTexture, vTexCoord).g;

    vec3 N = normalize(normal);
    vec3 V = normalize(cameraPosition - vPosition);
    vec3 R = reflect(-V, N);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, baseColor, metallic);

    vec3 lightLo = vec3(0.0);
    vec3 L = normalize(sunPosition - vPosition);
    vec3 H = normalize(V + L);

    float distance = length(sunPosition - vPosition);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = sunLuminance * attenuation * SUNLIGHT_INTENSITY;

    float NDF = DistributionGGX(N, H, roughness);
    float G   = GeometrySmith(N, V, L, roughness);
    vec3  F   = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3  numerator   = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3  specular    = numerator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;
    float NdotL = max(dot(N, L), 0.0);   

    lightLo += (kD * baseColor / PI + specular) * radiance * NdotL;

    vec3 result = lightLo + emissive * EMISSIVE_INTENSITY;
    FragColor = vec4(result, 1.0);
}