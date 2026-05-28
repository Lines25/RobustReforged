#include <omp.h>
#include <cmath>
#include "reforged_config.h"
#include "reforged_physics.h"

// #include "reforged_log.h"
// #include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(REFORGED_PHYSICS_USE_FAST_RSQRT) && (defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86))
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
#if defined(REFORGED_PHYSICS_USE_FAST_RSQRT) // If ARM with defined(REFORGED_PHYSICS_USE_FAST_RSQRT)
#warning "[REOFRGED PHYSICS] Can't use fast rsqrt - is NOT supported on ARM"
#endif
static inline float _rg_rsqrt(float x) {
	return 1/sqrt(fmaxf(x, 0.00001));
}
#endif

static inline float clamp01(float v) {
    return fminf(fmaxf(v, 0.0f), 1.0f);
}

// default - x86-64-v2 - sse4.2
//__attribute__((target_clones("avx2,default")))
REFORGED_API void IntegrateVelocitiesNative(float* __restrict__ vels, float* __restrict__ angVels, const PhysicsBodyData* __restrict__ data, int count, float dt, float gravX, float gravY) {
    #pragma omp parallel for if(count >= REFORGED_INTEGRATE_OMP_COUNT) schedule(static)
    for (int i = 0; i < count; ++i) {
        if (!data[i].isDynamic) continue;

        const int i2 = i << 1;
        const float im = data[i].invMass;

        vels[i2]   += (gravX * data[i].gravityScale + data[i].fx * im) * dt;
        vels[i2+1] += (gravY * data[i].gravityScale + data[i].fy * im) * dt;

        angVels[i] += data[i].invI * data[i].torque * dt;

        const float ld = clamp01(1.0f - dt * data[i].linDamp);
        const float ad = clamp01(1.0f - dt * data[i].angDamp);
        
        vels[i2] *= ld; 
        vels[i2+1] *= ld;
        angVels[i] *= ad;
    }
}

REFORGED_API void IntegratePositionsNative(float* __restrict__ positions, float* __restrict__ angles, float* __restrict__ vels, float* __restrict__ angVels, int count, float dt, float maxVel, float maxAngVel) {
    const float maxVelSq = maxVel * maxVel;
    const float maxAngVelSq = maxAngVel * maxAngVel;

    #pragma omp parallel for if(count >= REFORGED_INTEGRATE_OMP_COUNT) schedule(static)
    for (int i = 0; i < count; ++i) {
        const int i2 = i << 1;
        float vx = vels[i2];
        float vy = vels[i2+1];
        float av = angVels[i];

        const float vSqr = vx * vx + vy * vy;
        if (vSqr > maxVelSq) {
            const float ratio = maxVel / std::sqrt(vSqr);
            vx *= ratio; 
            vy *= ratio;
            vels[i2] = vx; 
            vels[i2+1] = vy;
        }

        if (av * av > maxAngVelSq) {
            av *= maxAngVel / std::abs(av);
            angVels[i] = av;
        }

        positions[i2] += vx * dt;
        positions[i2+1] += vy * dt;
        angles[i] += av * dt;
    }
}

#ifdef __cplusplus
}
#endif

