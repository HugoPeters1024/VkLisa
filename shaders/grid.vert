#version 460

layout(location = 0) in vec4 vPos;
layout(location = 1) in vec4 vColor;

layout(push_constant) uniform Constants {
    uint nrTriangles;
    uint nrInstanceWidth;
    uint nrInstanceHeight;
} constants;

layout(location = 0) out vec2 uv;
layout(location = 1) out vec4 color;

void main() {
    uint totalInstances = constants.nrInstanceWidth * constants.nrInstanceHeight;
    uint triangleId = gl_VertexIndex / 3;
    uint instanceId = (totalInstances * triangleId) / constants.nrTriangles;
    float instanceIdx = mod(instanceId, constants.nrInstanceWidth);
    float instanceIdy = instanceId / constants.nrInstanceHeight;

    // [0 .. {width,height}]
    vec2 offs = vec2(instanceIdx, instanceIdy);

    // [0 .. 1]
    vec2 normalizedPos = (vPos.xy + offs) / vec2(constants.nrInstanceWidth, constants.nrInstanceHeight);

    gl_Position = vec4(normalizedPos * 2 - 1, 0.0f, 1.0f);
    uv = normalizedPos;
    color = vColor;
}
