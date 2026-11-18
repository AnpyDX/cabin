/** Post-Processing: Bloom Down Sampling Stage.
* - ref: <https://zhuanlan.zhihu.com/p/525500877>
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
#![use("common/bloom.su")]
out vec4 FragColor;

in vec2 vTexCoord;

uniform vec2 texelSizePrev;
uniform sampler2D texturePrev;

void main() {
    vec3 color = GaussNxN(texturePrev, vTexCoord, 5, texelSizePrev, 1.0);
    FragColor = vec4(color, 1.0);
}