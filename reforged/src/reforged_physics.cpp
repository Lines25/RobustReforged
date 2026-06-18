#include <omp.h>
#include <cmath>
#include "reforged_config.h"
#include "reforged_physics.h"

#ifdef __cplusplus
extern "C" {
#endif

// Компілятор (з -ffast-math) автоматично замінить це на апаратну інструкцію RSQRT (для векторів - rsqrtps).
// Це набагато безпечніше і швидше для SIMD циклів, ніж ручні скалярні інструкції _mm_set_ss.
static inline float _rg_rsqrt(float x) {
    return 1.0f / std::sqrt(std::fmax(x, 0.00001f));
}

static inline float clamp01(float v) {
    return std::fmin(std::fmax(v, 0.0f), 1.0f);
}

REFORGED_API void IntegrateVelocitiesNative(
    float* __restrict__ vels, 
    float* __restrict__ angVels, 
    const PhysicsBodyData* __restrict__ data, 
    int count, 
    float dt, 
    float gravX, 
    float gravY) 
{
    // Звичайний parallel for (без simd). 
    // Завдяки 'continue' ми оберігаємо пам'ять від непотрібного перезапису для статичних тіл (економить Cache Bandwidth).
    #pragma omp parallel for if(count >= REFORGED_INTEGRATE_OMP_COUNT) schedule(static)
    for (int i = 0; i < count; ++i) {
        
        // Якщо тіло статичне - просто ігноруємо.
        if (!data[i].isDynamic) continue;

        const int i2 = i * 2; // Швидше, ніж зсув (i << 1), хоча компілятор сам оптимізує
        const float im = data[i].invMass;

        // Зчитуємо старі швидкості локально
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

        // Записуємо результати 1 раз
        vels[i2]   = vx;
        vels[i2+1] = vy;
        angVels[i] = av;
    }
}

REFORGED_API void IntegratePositionsNative(
    float* __restrict__ positions, 
    float* __restrict__ angles, 
    float* __restrict__ vels, 
    float* __restrict__ angVels, 
    int count, 
    float dt, 
    float maxVel, 
    float maxAngVel) 
{
    const float maxVelSq = maxVel * maxVel;

    // Тут ми використовуємо SIMD, оскільки код всередині БЕЗРОЗГАЛУЖЕНИЙ (branchless).
    // Процесор братиме по 4/8 тіл за раз і опрацьовуватиме їх паралельно векторними регістрами!
    #pragma omp parallel for simd if(count >= REFORGED_INTEGRATE_OMP_COUNT) schedule(static)
    for (int i = 0; i < count; ++i) {
        const int i2 = i * 2;
        
        float vx = vels[i2];
        float vy = vels[i2+1];
        float av = angVels[i];

        const float vSqr = vx * vx + vy * vy;
        
        // Branchless ліміт лінійної швидкості (через тернарний оператор компілятор зробить CMOV/VBLENDPS)
        const float ratio = (vSqr > maxVelSq) ? (maxVel * _rg_rsqrt(vSqr)) : 1.0f;
        vx *= ratio;
        vy *= ratio;

        vels[i2]   = vx;
        vels[i2+1] = vy;

        // Branchless ліміт кутової швидкості
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
