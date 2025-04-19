#version 460

#define Thickness 0.008f

layout(location = 0) out vec4 OutColor;

in vec4 gl_FragCoord;
layout(location = 0) in vec3 FragColor;
layout(push_constant) uniform push_constants {
    vec2 Resolution;
    vec2 MousPos;
    float Time;
} PushConstants;

void main() {
    //    vec2 Coord = gl_FragCoord.xy / (PushConstants.Resolution / 2.0f);
    //    Coord.x = Coord.x - 1.0f;
    //    Coord.y = 1.0f - Coord.y;
    //
    //    float Horizontal = 1.0f - step(Thickness / 2.0f, abs(Coord.x));
    //    float Vertical = 1.0f - step(Thickness / 2.0f, abs(Coord.y));
    //
    //    vec3 Color = FragColor;
    //    vec3 DebugColor = vec3(0.108, 0.026, 0.074);
    //
    //    Color = (1.0f - Vertical) * Color + Vertical * DebugColor;
    //    Color = (1.0f - Horizontal) * Color + Horizontal * DebugColor;
    //
    //    OutColor = vec4(Color, 1.0f);
    OutColor = vec4(FragColor, 1.0);
}
