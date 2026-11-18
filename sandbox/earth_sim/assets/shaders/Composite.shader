/* Texture Color Composition Shader */

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

uniform float factor;
uniform sampler2D textureInput;

void main() {
    vec3 color = texture(textureInput, vTexCoord).rgb;
    FragColor = vec4(factor * color, 1.0);
}