#version 460

layout(location = 0) in vec2 uv;

layout(binding = 0) uniform sampler2D texInput;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(texture(texInput, uv).xyz, 1.0f);
}

