/* Sun Light Rendering */

#![version("430 core")]

#![vertex]
layout (location = 0) in vec3 aPosition;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPosition, 1.0);
}

#![fragment]
out vec4 FragColor;

uniform vec3 sunLuminance;

void main() {
    FragColor = vec4(sunLuminance, 1.0);
}