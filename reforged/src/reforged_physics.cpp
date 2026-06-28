#include <omp.h>
#include <cmath>
#include "reforged_config.h"
#include "reforged_physics.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline float _rg_rsqrt(float x) {
    return 1.0f / std::sqrt(std::fmax(x, 0.00001f));
}

static inline float clamp01(float v) {
    return std::fmin(std::fmax(v, 0.0f), 1.0f);
}

REFORGED_API void IntegrateVelocitiesNative(float* __restrict__ vels, float* __restrict__ angVels, const PhysicsBodyData* __restrict__ data, int count, float dt, float gravX, float gravY) {
    #pragma omp parallel for if(count >= REFORGED_INTEGRATE_OMP_COUNT) schedule(static)
    for (int i = 0; i < count; ++i) {
        
        if (!data[i].isDynamic) continue;

        const int i2 = i * 2;
        const float im = data[i].invMass;

        float vx = vels[i2];
        float vy = vels[i2+1];
        float av = angVels[i];

        vx += (gravX * data[i].gravityScale + data[i].fx * im) * dt;
        vy += (gravY * data[i].gravityScale + data[i].fy * im) * dt;
        av += data[i].invI * data[i].torque * dt;

        const float ld = clamp01(1.0f - dt * data[i].linDamp);
        const float ad = clamp01(1.0f - dt * data[i].angDamp);
        
        vx *= ld; 
        vy *= ld;
        av *= ad;

        vels[i2]   = vx;
        vels[i2+1] = vy;
        angVels[i] = av;
    }
}

REFORGED_API void IntegratePositionsNative(float* __restrict__ positions, float* __restrict__ angles, float* __restrict__ vels, float* __restrict__ angVels, int count, float dt, float maxVel, float maxAngVel) {
    const float maxVelSq = maxVel * maxVel;

    #pragma omp parallel for simd if(count >= REFORGED_INTEGRATE_OMP_COUNT) schedule(static)
    for (int i = 0; i < count; ++i) {
        const int i2 = i * 2;
        
        float vx = vels[i2];
        float vy = vels[i2+1];
        float av = angVels[i];

        const float vSqr = vx * vx + vy * vy;
        
        const float ratio = (vSqr > maxVelSq) ? (maxVel * _rg_rsqrt(vSqr)) : 1.0f;
        vx *= ratio;
        vy *= ratio;

        vels[i2]   = vx;
        vels[i2+1] = vy;

        const float clampedAv = std::fmin(std::abs(av), maxAngVel);
        av = std::copysign(clampedAv, av);
        angVels[i] = av;

        positions[i2]   += vx * dt;
        positions[i2+1] += vy * dt;
        angles[i]       += av * dt;
    }
}

#ifdef __cplusplus
}
#endif
