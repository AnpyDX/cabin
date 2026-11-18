/** Atmosphere Transmittance LUT Rendering.
* - ref: <https://zhuanlan.zhihu.com/p/595576594>
*/

#![version("430 core")]

#![vertex]
layout (location = 0) in vec3 aPosition;
layout (location = 2) in vec2 aTexCoord;

out vec2 vTexCoord;

void main() {
    gl_Position = vec4(aPosition.xy, 0.0, 1.0);
    vTexCoord = aTexCoord;
}

#![fragment]
#![use("common/atmos.su")]
out vec4 FragColor;

in vec2 vTexCoord;

void main() {
    vec3 params = UVToTransmittanceParams(vTexCoord);

    float mu = params.x;
    float d  = params.y;
    float r  = params.z;
    vec3 p1 = vec3(0.0, r, 0.0);
    vec3 p2 = vec3(d * max(0.0, sqrt(1 - mu * mu)), r + d * mu, 0.0);

    vec3 transmittance = Transmittance(vec3(0.0), p1, p2);

    FragColor = vec4(transmittance, 1.0);
}