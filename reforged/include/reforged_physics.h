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

typedef struct PhysicsBodyData {
    float PosX, PosY;
    float VelX, VelY;
    float AngVel;
    float Friction;
    float AngDamping;
    float InvMass;
    float InvI;
    float ForceX, ForceY;
    float Torque;
} PhysicsBodyData;

REFORGED_API void IntegrateAllParallel(
    float* positions, float* angles,
    float* vels, float* angVels,
    const float* forces, const float* torques,
    const float* invMasses, const float* invI,
    const float* friction, const float* angDamp,
    int count, float dt, float maxVel, float maxAngVel);

REFORGED_API void StepPhysicsParallel(PhysicsBodyData* bodies, int count, float frameTime);
REFORGED_API void IntegratePositionsNative(float* positions, float* angles, float* vels, float* angVels, int count, float dt, float maxVel, float maxAngVel);

#ifdef __cplusplus
}
#endif
