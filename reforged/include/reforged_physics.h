#pragma once

#if defined(_MSC_VER)
    #define REFORGED_API __declspec(dllexport)
#elif defined(__GNUC__) || defined(__clang__)
    #define REFORGED_API __attribute__((visibility("default")))
#else
    #define REFORGED_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 1)
struct PhysicsBodyData {
    float fx, fy, torque;
    float invMass, invI;
    float linDamp, angDamp;
    float gravityScale;
};
#pragma pack(pop)

REFORGED_API void IntegrateAllParallel(float* __restrict__ positions, float* __restrict__ angles, float* __restrict__ vels, float* __restrict__ angVels, const PhysicsBodyData* __restrict__ data, int count, float dt, float gravX, float gravY, float maxVel, float maxAngVel);

#ifdef __cplusplus
}
#endif
