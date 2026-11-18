/** Post-Processing: Tone Mapping & Gamma Correction.
* - ref: <https://zhuanlan.zhihu.com/p/662140618>
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
out vec4 FragColor;

in vec2 vTexCoord;

uniform sampler2D textureInput;

/// ref: <https://zhuanlan.zhihu.com/p/662140618>
vec3 Log2Tonemap(vec3 color) {
    vec3 logColor = log2(max(color, 0.0001));
    vec3 compressed = exp2(logColor * 0.33) * 1.4938 - 0.7;
    vec3 tonemapped = mix(color, compressed, step(0.3, color));
    return clamp(tonemapped, 0.0, 1.0);
}

vec3 GammaCorrection(vec3 color) {
    return pow(color, vec3(1.0 / 2.2));
}

void main() {
    vec3 color = texture(textureInput, vTexCoord).rgb;
    color = Log2Tonemap(color);
    color = GammaCorrection(color);
    FragColor = vec4(color, 1.0);
}