/** Model's PBR Shader */

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
#![use("PBR.utils")]
out vec4 FragColor;

in vec3 vPosition;
in vec3 vNormal;
in vec2 vTexCoord;

uniform vec3 cameraPosition;
uniform vec3 lightPositions[4];
uniform vec3 lightColor;

uniform int normalTexMarker;
uniform int baseColorTexMarker;
uniform int metallicRoughnessTexMarker;
uniform int occulsionTexMarker;

uniform vec4 baseColorFactor;
uniform float metallicFactor;
uniform float roughnessFactor;

uniform sampler2D normalTexture;
uniform sampler2D baseColorTexture;
uniform sampler2D metallicRoughnessTexture;
uniform sampler2D occlusionTexture;

uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D BRDFLUTMap;

vec3 getNormalFromMap() {
    vec3 tangentNormal = texture(normalTexture, vTexCoord).rgb * 2.0 - 1.0;

    vec3 Q1  = dFdx(vPosition);
    vec3 Q2  = dFdy(vPosition);
    vec2 st1 = dFdx(vTexCoord);
    vec2 st2 = dFdy(vTexCoord);

    vec3 N   = normalize(vNormal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

void main() {
    vec3 normal = vNormal;
    if (normalTexMarker == 1)
        normal = getNormalFromMap();

    vec3 baseColor;
    if (baseColorTexMarker == 1) {
        baseColor = texture(baseColorTexture, vTexCoord).rgb;
    }
    else
        baseColor = baseColorFactor.rgb;

    float metallic, roughness;
    if (metallicRoughnessTexMarker == 1) {
        metallic = texture(metallicRoughnessTexture, vTexCoord).b;
        roughness = texture(metallicRoughnessTexture, vTexCoord).g;
    }
    else {
        metallic = metallicFactor;
        roughness = roughnessFactor;
    }
    
    float occlusion = 1.0f;
    if (occulsionTexMarker == 1)
        occlusion = texture(occlusionTexture, vTexCoord).r;

    vec3 N = normalize(normal);
    vec3 V = normalize(cameraPosition - vPosition);
    vec3 R = reflect(-V, N);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, baseColor, metallic);

    vec3 lightLo = vec3(0.0);
    for (int i = 0; i < 4; i++) {
        vec3 L = normalize(lightPositions[i] - vPosition);
        vec3 H = normalize(V + L);

        float distance = length(lightPositions[i] - vPosition);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lightColor * attenuation;

        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;
        float NdotL = max(dot(N, L), 0.0);   

        lightLo += (kD * baseColor / PI + specular) * radiance * NdotL;
    }

    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse = irradiance * baseColor;

    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilterColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;   
    vec2 envBRDF  = texture(BRDFLUTMap, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilterColor * (F * envBRDF.x + envBRDF.y);

    vec3 ambient = (kD * diffuse +  specular) * occlusion;

    vec3 result = ambient + lightLo;
    result = result / (result + vec3(1.0));
    result = pow(result, vec3(1.0 / 2.2));
    
    FragColor = vec4(result, 1.0f);
}