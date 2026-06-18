#pragma once

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

// 36 bytes
struct PhysicsBodyData {
    float fx;
    float fy;
    float torque;
    float invMass;
    float invI;
    float linDamp;
    float angDamp;
    float gravityScale;
    int32_t isDynamic;
};

REFORGED_API void IntegrateVelocitiesNative(float* __restrict__ vels, float* __restrict__ angVels, const PhysicsBodyData* __restrict__ data, int count, float dt, float gravX, float gravY);
REFORGED_API void IntegratePositionsNative(float* __restrict__ positions, float* __restrict__ angles, float* __restrict__ vels, float* __restrict__ angVels, int count, float dt, float maxVel, float maxAngVel);

#ifdef __cplusplus
}
#endif
