#pragma once

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 1)
struct PhysicsBodyData {
    float fx, fy, torque;
    float invMass, invI;
    float linDamp, angDamp;
    float gravityScale;
    uint8_t isDynamic;
};
#pragma pack(pop)

REFORGED_API void IntegrateVelocitiesNative(float* __restrict__ vels, float* __restrict__ angVels, const PhysicsBodyData* __restrict__ data, int count, float dt, float gravX, float gravY);
REFORGED_API void IntegratePositionsNative(float* __restrict__ positions, float* __restrict__ angles, float* __restrict__ vels, float* __restrict__ angVels, int count, float dt, float maxVel, float maxAngVel);

#ifdef __cplusplus
}
#endif
