#pragma once
#include "reforged_config.h"
#include <stdint.h>

// 36 bytes
struct NativeVelocityConstraintPoint {
    float relVelAx, relVelAy; // 0
    float relVelBx, relVelBy; // 8
    float normalImpulse; // 16
    float tangentImpulse; // 20
    float normalMass; // 24
    float tangentMass; // 28
    float velocityBias; // 32
};
static_assert(sizeof(NativeVelocityConstraintPoint) == 36, "NativeVelocityConstraintPoint size mismatch");

// 156 bytes
struct NativeContactVelocityConstraint {
    int32_t contactIndex; // 0
    int32_t indexA; // 4
    int32_t indexB; // 8
    NativeVelocityConstraintPoint points[2]; // 12
    float normalX, normalY; // 84
    float normalMass[4]; // Vector4; 92
    float k[4]; // Vector4; 108
    float invMassA; // 124
    float invMassB; // 128
    float invIA; // 132
    float invIB; // 136
    float friction; // 140
    float restitution; // 144
    float tangentSpeed; // 148
    int32_t pointCount; // 152
};
static_assert(sizeof(NativeContactVelocityConstraint) == 156, "NativeContactVelocityConstraint size mismatch");

#ifdef __cplusplus
extern "C" {
#endif

REFORGED_API void SolveVelocityConstraintsNative(
    NativeContactVelocityConstraint* __restrict__ constraints,
    int count,
    float* __restrict__ linearVelocities,  // [x0,y0,x1,y1,...]
    float* __restrict__ angularVelocities,
    int bodyOffset
);

REFORGED_API void WarmStartNative(
    NativeContactVelocityConstraint* constraints,
    int count,
    float* linearVelocities,
    float* angularVelocities,
    int bodyOffset
);

#ifdef __cplusplus
}
#endif

