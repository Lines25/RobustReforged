#include <omp.h>
#include <cmath>
#include "reforged_config.h"
#include "reforged_physics.h"

// #include "reforged_log.h"
// #include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef REFORGED_PHYSICS_USE_FAST_RSQRT
#include <immintrin.h>
static inline float _rg_rsqrt(float x) {
	x = fmaxf(x, 0.00001);
    __m128 v = _mm_set_ss(x);
    __m128 r = _mm_rsqrt_ss(v);
    __m128 h = _mm_set_ss(0.5f * x);
    __m128 t = _mm_set_ss(1.5f);
    r = _mm_mul_ss(r, _mm_sub_ss(t, _mm_mul_ss(h, _mm_mul_ss(r, r))));
    return _mm_cvtss_f32(r);
}
#else
static inline float _rg_rsqrt(float x) {
	return 1/sqrt(fmaxf(x, 0.00001));
}
#endif

// default - x86-64-v2 - sse4.2
__attribute__((target_clones("avx2,default")))
REFORGED_API void IntegrateAllParallel(float* __restrict__ positions, float* __restrict__ angles, float* __restrict__ vels, float* __restrict__ angVels, const float* __restrict__ forces, const float* __restrict__ torques, const float* __restrict__ invMasses, const float* __restrict__ invI, const float* __restrict__ friction, const float* __restrict__ angDamp, int count, float dt, float maxVel, float maxAngVel) {
    const float maxVelSq = maxVel * maxVel;

    // char msg[1024];
    // snprintf(msg, 1024, "(PHYSICS) TICK ! Count: %d", count);
    // reforged_log(msg);

    #pragma omp parallel for if(count >= REFORGED_INTEGRATE_OMP_COUNT) schedule(static)
    for (int i = 0; i < count; ++i) {
        const float im = invMasses[i];
        
        const int i2 = i << 1;
        const float dtIm = dt * im;

        float vx = vels[i2] + forces[i2] * dtIm;
        float vy = vels[i2+1] + forces[i2+1] * dtIm;

        const float ld = 1.f / (1.f + dt * friction[i]);
        vx *= ld;
        vy *= ld;

        const float velSq = vx * vx + vy * vy;
        if (__builtin_expect(velSq > maxVelSq, 0)) {
            const float ratio = maxVel * _rg_rsqrt(velSq);
            vx *= ratio;
            vy *= ratio;
        }

        vels[i2] = vx;
        vels[i2+1] = vy;
        positions[i2] += vx * dt;
        positions[i2+1] += vy * dt;

        const float av = (angVels[i] + torques[i] * invI[i] * dt) * (1.f / (1.f + dt * angDamp[i]));
        angVels[i] = av;
        angles[i] += av * dt;
    }
}

REFORGED_API void StepPhysicsParallel(PhysicsBodyData* bodies, int count, float frameTime) {
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < count; ++i) {
        if (bodies[i].InvMass <= 0.0f) continue;

		// Linear
        bodies[i].VelX += (bodies[i].ForceX * bodies[i].InvMass) * frameTime;
        bodies[i].VelY += (bodies[i].ForceY * bodies[i].InvMass) * frameTime;

        float linDamp = 1.0f / (1.0f + frameTime * bodies[i].Friction);
        bodies[i].VelX *= linDamp;
        bodies[i].VelY *= linDamp;

		// Rotation
        bodies[i].AngVel += (bodies[i].Torque * bodies[i].InvI) * frameTime;

        float angDamp = 1.0f / (1.0f + frameTime * bodies[i].AngDamping);
        bodies[i].AngVel *= angDamp;
    }
}

// REFORGED_API void IntegratePositionsNative(float* positions, float* angles, float* vels, float* angVels, int count, float dt, float maxVel, float maxAngVel)  {
//     float maxVelSq = maxVel * maxVel;
//     float maxAngVelSq = maxAngVel * maxAngVel;
// 
//     #pragma omp parallel for schedule(static)
//     for (int i = 0; i < count; ++i) {
//         float vx = vels[i*2];
//         float vy = vels[i*2 + 1];
//         float velSq = vx*vx + vy*vy;
// 
//         if (velSq > maxVelSq && velSq > 0.0001f) {
//             float ratio = maxVel / std::sqrt(velSq);
//             vx *= ratio;
//             vy *= ratio;
//             vels[i*2] = vx;
//             vels[i*2 + 1] = vy;
//         }
// 
//         float av = angVels[i];
//         if (av * av > maxAngVelSq) {
//             av = (av > 0 ? maxAngVel : -maxAngVel);
//             angVels[i] = av;
//         }
// 
//         positions[i*2] += vx * dt;
//         positions[i*2 + 1] += vy * dt;
//         angles[i] += av * dt;
//     }
// }

#ifdef __cplusplus
}
#endif

