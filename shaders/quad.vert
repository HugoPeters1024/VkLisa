#version 460

layout(location = 0) in vec2 vPos;
layout(location = 1) in vec4 vColor;

vec2 positions[3] = vec2[](
    vec2(-1.0f, -1.0f),
    vec2(3.0f, -1.0f),
    vec2(-1.0f, 3.0f)
);

layout(location = 0) out vec2 uv;
layout(location = 1) out vec4 color;

void main() {
    gl_Position = vec4(vPos, 0.0f, 1.0f);
    uv = (vPos + 1.0f) * 0.5f;
    color = vColor;
}
