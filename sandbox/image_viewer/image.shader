#![version("430 core")]

#![vertex]
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 texCoord;

uniform mat4 model;
uniform mat4 projection;

void main() {
    gl_Position = projection * model * vec4(aPos, 1.0, 1.0);
    texCoord = aTexCoord;
}

#![fragment]
in vec2 texCoord;
out vec4 FragColor;

uniform sampler2D texture0;

void main() {
    FragColor = texture(texture0, texCoord);
}