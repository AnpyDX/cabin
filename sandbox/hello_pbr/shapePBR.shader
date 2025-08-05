/** Shape's PBR Shader */

#![version("430 core")]

#![vertex]
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;

out vec3 vPosition;
out vec3 vNormal;

void main() {
    gl_Position = projection * view * model * vec4(aPosition, 1.0);
    vPosition = vec3(model * vec4(aPosition, 1.0));
    vNormal = normalMatrix * aNormal;
}

#![fragment]
#![use("PBR.utils")]
out vec4 FragColor;

in vec3 vPosition;
in vec3 vNormal;

uniform vec3 cameraPosition;
uniform vec3 lightPositions[4];
uniform vec3 lightColor;

uniform vec3 baseColorFactor;
uniform float metallicFactor;
uniform float roughnessFactor;
uniform float occlusionFactor;

uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D BRDFLUTMap;

void main() {
    vec3 N = normalize(vNormal);
    vec3 V = normalize(cameraPosition - vPosition);
    vec3 R = reflect(-V, N);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, baseColorFactor, metallicFactor);

    vec3 lightLo = vec3(0.0);
    for (int i = 0; i < 4; i++) {
        vec3 L = normalize(lightPositions[i] - vPosition);
        vec3 H = normalize(V + L);

        float distance = length(lightPositions[i] - vPosition);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lightColor * attenuation;

        float NDF = DistributionGGX(N, H, roughnessFactor);
        float G   = GeometrySmith(N, V, L, roughnessFactor);
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallicFactor;
        float NdotL = max(dot(N, L), 0.0);   

        lightLo += (kD * baseColorFactor / PI + specular) * radiance * NdotL;
    }

    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughnessFactor);
    
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallicFactor;

    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse = irradiance * baseColorFactor;

    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilterColor = textureLod(prefilterMap, R,  roughnessFactor * MAX_REFLECTION_LOD).rgb;   
    vec2 envBRDF  = texture(BRDFLUTMap, vec2(max(dot(N, V), 0.0), roughnessFactor)).rg;
    vec3 specular = prefilterColor * (F * envBRDF.x + envBRDF.y);

    vec3 ambient = (kD * diffuse +  specular) * occlusionFactor;

    vec3 result = ambient + lightLo;
    result = result / (result + vec3(1.0));
    result = pow(result, vec3(1.0 / 2.2));

    FragColor = vec4(result, 1.0);
}