#pragma once
#include "reforged_config.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 1)

// 36 bytes
struct NativeVelocityConstraintPoint {
    float relVelAx, relVelAy;
    float relVelBx, relVelBy;
    float normalImpulse;
    float tangentImpulse;
    float normalMass;
    float tangentMass;
    float velocityBias;
};

// 152 bytes
struct NativeContactVelocityConstraint {
    int32_t contactIndex;
    int32_t indexA;
    int32_t indexB;
    NativeVelocityConstraintPoint points[2]; // FixedArray2<VelocityConstraintPoint> Points;
    float normalX, normalY;
    float normalMass[4]; // Vector4
    float k[4];          // Vector4
    float invMassA;
    float invMassB;
    float invIA;
    float invIB;
    float friction;
    float restitution;
    float tangentSpeed;
    int32_t pointCount;
};

#pragma pack(pop)

REFORGED_API void SolveVelocityConstraintsNative(
    NativeContactVelocityConstraint* __restrict__ constraints,
    int count,
    float* __restrict__ linearVelocities,
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
