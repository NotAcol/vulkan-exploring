#version 460

layout(location = 0) out vec4 OutColor;

in vec4 gl_FragCoord;
layout(location = 0) in vec3 InColor;
layout(push_constant) uniform push_constants {
    vec2 Resolution;
    vec2 MousPos;
    float Time;
} PushConstants;

void main() {
    vec2 Coord = gl_FragCoord.xy / (PushConstants.Resolution * 0.5f) - vec2(1.0);

    vec3 Color = vec3(smoothstep(-0.35, 0.5, Coord.x));
    OutColor = vec4(Color + InColor, 1.0);
}
