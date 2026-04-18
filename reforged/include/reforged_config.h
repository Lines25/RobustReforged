#pragma once

// If defined - fast reverse SQRT will be used (Newton-Raphson with 1 step, ~23bit accuracy)
// If not defined - standart 1/sqrt(x) will be used. Very accurate but can vary in speed, depending on platform
#define REFORGED_PHYSICS_USE_FAST_RSQRT

// Minimum amount of objects that went into IntegrateAllParallel
// After which function will use omp to speed up (parallel onto cores&threads)
#define REFORGED_INTEGRATE_OMP_COUNT 32
