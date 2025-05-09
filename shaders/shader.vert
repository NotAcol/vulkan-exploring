#version 460

#define PI 3.1415926538f

layout(location = 0) in vec3 InPosition;
layout(location = 1) in vec3 InColor;
layout(push_constant) uniform push_constants {
    vec2 Resolution;
    vec2 MousPos;
    float Time;
} PushConstants;

layout(location = 0) out vec3 OutColor;

void main() {
    float TimeToAngle = PI * (PushConstants.Time / 1000.0f);

    float AngZ = (10) * PI / 180.f;
    float AngX = (15) * PI / 180.f;
    float AngY = TimeToAngle / 2.0; //(0.0) * PI / 180.f;

    mat3 RotationZ = mat3(cos(AngZ), -sin(AngZ), 0, sin(AngZ), cos(AngZ), 0, 0, 0, 1.0);

    mat3 RotationX = mat3(1., 0., 0., 0., cos(AngX), -sin(AngX), 0., sin(AngX), cos(AngX));

    mat3 RotationY = mat3(cos(AngY), 0., sin(AngY), 0., 1., 0., -sin(AngY), 0., cos(AngY));

    gl_Position = vec4((RotationX * RotationY * RotationZ * (InPosition / 2.0)) + vec3(0., 0., 0.5), 1.0);
    OutColor = InColor;
}
