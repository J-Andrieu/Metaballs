#version 330 core

uniform sampler2D tex;
uniform vec2 tex_size;

layout (location = 0) out vec4 out_color;

void main() {
    out_color = texture(tex, gl_FragCoord.xy / tex_size);
}