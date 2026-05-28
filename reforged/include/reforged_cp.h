#pragma once
#include "reforged_config.h"
#include <stdint.h>

struct NativeVelocityConstraintPoint {
    float relVelAx, relVelAy;
    float relVelBx, relVelBy;
    float normalImpulse;
    float tangentImpulse;
    float normalMass;
    float tangentMass;
    float velocityBias;
    float _pad;
};
static_assert(sizeof(NativeVelocityConstraintPoint) == 40);

struct NativeContactVelocityConstraint {
    int32_t contactIndex;
    int32_t indexA;
    int32_t indexB;
    int32_t pointCount;
    NativeVelocityConstraintPoint points[2];
    float normalX, normalY;
    float normalMass[4]; // Vector4
    float k[4];          // Vector4
    float invMassA, invMassB;
    float invIA, invIB;
    float friction, restitution, tangentSpeed;
    float _pad;
};

#ifdef __cplusplus
extern "C" {
#endif

REFORGED_API void SolveVelocityConstraintsNative(
    NativeContactVelocityConstraint* constraints,
    int count,
    float* linearVelocities,  // [x0,y0,x1,y1,...]
    float* angularVelocities,
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
