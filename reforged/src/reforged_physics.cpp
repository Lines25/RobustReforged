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
REFORGED_API void IntegrateAllParallel(float* __restrict__ positions, float* __restrict__ angles, float* __restrict__ vels, float* __restrict__ angVels, const PhysicsBodyData* __restrict__ data, int count, float dt, float gravX, float gravY, float maxVel, float maxAngVel) {
    const float maxVelSq = maxVel * maxVel;
    const float maxAngVelSq = maxAngVel * maxAngVel;

    #pragma omp parallel for if(count >= REFORGED_INTEGRATE_OMP_COUNT) schedule(static)
    for (int i = 0; i < count; ++i) {
        const auto& b = data[i];
        // if (b.invMass <= 0.0f) continue;

        int i2 = i << 1;
        float vx = vels[i2];
        float vy = vels[i2+1];
        float av = angVels[i];

        vx += (b.fx * b.invMass + gravX * b.gravityScale) * dt;
        vy += (b.fy * b.invMass + gravY * b.gravityScale) * dt;
        av += (b.torque * b.invI) * dt;

        float ld = std::max(1.0f - dt * b.linDamp, 0.0f);
        float ad = std::max(1.0f - dt * b.angDamp, 0.0f);
        vx *= ld; vy *= ld; av *= ad;

		// limits
        float vSq = vx * vx + vy * vy;
        if (vSq > maxVelSq) {
            float ratio = maxVel * _rg_rsqrt(vSq);
            vx *= ratio; vy *= ratio;
        }
        if (av * av > maxAngVelSq) {
            av = (av > 0.0f) ? maxAngVel : -maxAngVel;
        }

        vels[i2] = vx; vels[i2+1] = vy;
        angVels[i] = av;
        positions[i2] += vx * dt;
        positions[i2+1] += vy * dt;
        angles[i] += av * dt;
    }
}

#ifdef __cplusplus
}
#endif
