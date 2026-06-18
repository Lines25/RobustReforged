#include "reforged_cp.h"
#include <algorithm>
#include <cmath>

// Cross(vec, vec) -> scalar
static inline float cross_vv(float ax, float ay, float bx, float by) {
    return ax * by - ay * bx;
}

// Cross(scalar, vec) -> vec
static inline void cross_sv(float s, float rx, float ry, float& ox, float& oy) {
    ox = -s * ry;
    oy =  s * rx;
}

static inline void mul_mat2_vec2(const float m[4], float ax, float ay, float& rx, float& ry) {
    rx = m[0] * ax + m[1] * ay;
    ry = m[2] * ax + m[3] * ay;
}

// apply impulse to body pair
// this are fuckin' HUGE params
static inline void _rg_apply_impulse_pair(float P1x, float P1y, float P2x, float P2y, float cp1_rAx, float cp1_rAy, float cp2_rAx, float cp2_rAy, float cp1_rBx, float cp1_rBy, float cp2_rBx, float cp2_rBy, float mA, float iA, float mB, float iB, float& vAx, float& vAy, float& wA, float& vBx, float& vBy, float& wB) {
    const float Psum_x = P1x + P2x;
    const float Psum_y = P1y + P2y;

    vAx -= Psum_x * mA;
    vAy -= Psum_y * mA;
    wA  -= iA * (cross_vv(cp1_rAx, cp1_rAy, P1x, P1y) + cross_vv(cp2_rAx, cp2_rAy, P2x, P2y));
    vBx += Psum_x * mB;
    vBy += Psum_y * mB;
    wB  += iB * (cross_vv(cp1_rBx, cp1_rBy, P1x, P1y) + cross_vv(cp2_rBx, cp2_rBy, P2x, P2y));
}
#ifdef __cplusplus
extern "C" {
#endif

//SharedPhysicsSystem.WarmStart()
REFORGED_API void WarmStartNative(NativeContactVelocityConstraint* __restrict__ constraints, int count, float* __restrict__ linearVelocities, float* __restrict__ angularVelocities, int bodyOffset) {
    for (int i = 0; i < count; ++i) {
        NativeContactVelocityConstraint& vc = constraints[i];

        const int iaIdx = bodyOffset + vc.indexA;
        const int ibIdx = bodyOffset + vc.indexB;

        float vAx_val = linearVelocities[iaIdx * 2];
        float vAy_val = linearVelocities[iaIdx * 2 + 1];
        float wA_val  = angularVelocities[iaIdx];
        float vBx_val = linearVelocities[ibIdx * 2];
        float vBy_val = linearVelocities[ibIdx * 2 + 1];
        float wB_val  = angularVelocities[ibIdx];

        const float nx = vc.normalX;
        const float ny = vc.normalY;
        const float tx = ny;
        const float ty = -nx;

        const float invMassA = vc.invMassA;
        const float invMassB = vc.invMassB;
        const float invIA    = vc.invIA;
        const float invIB    = vc.invIB;

        for (int j = 0; j < vc.pointCount; ++j) {
            NativeVelocityConstraintPoint& vcp = vc.points[j];

            const float Px = nx * vcp.normalImpulse + tx * vcp.tangentImpulse;
            const float Py = ny * vcp.normalImpulse + ty * vcp.tangentImpulse;

            wA_val  -= invIA * cross_vv(vcp.relVelAx, vcp.relVelAy, Px, Py);
            vAx_val -= Px * invMassA;
            vAy_val -= Py * invMassA;

            wB_val  += invIB * cross_vv(vcp.relVelBx, vcp.relVelBy, Px, Py);
            vBx_val += Px * invMassB;
            vBy_val += Py * invMassB;
        }

        linearVelocities[iaIdx * 2]     = vAx_val;
        linearVelocities[iaIdx * 2 + 1] = vAy_val;
        angularVelocities[iaIdx]        = wA_val;
        linearVelocities[ibIdx * 2]     = vBx_val;
        linearVelocities[ibIdx * 2 + 1] = vBy_val;
        angularVelocities[ibIdx]        = wB_val;
    }
}

//SharedPhysicsSystem.SolveVelocityConstraints()
REFORGED_API void SolveVelocityConstraintsNative(NativeContactVelocityConstraint* constraints, int count, float* linearVelocities, float* angularVelocities, int bodyOffset) {
    for (int i = 0; i < count; ++i) {
        NativeContactVelocityConstraint& vc = constraints[i];

        const int iaIdx = bodyOffset + vc.indexA;
        const int ibIdx = bodyOffset + vc.indexB;

        float& vAx = linearVelocities[iaIdx * 2];
        float& vAy = linearVelocities[iaIdx * 2 + 1];
        float& wA  = angularVelocities[iaIdx];
        float& vBx = linearVelocities[ibIdx * 2];
        float& vBy = linearVelocities[ibIdx * 2 + 1];
        float& wB  = angularVelocities[ibIdx];

        const float nx      = vc.normalX;
        const float ny      = vc.normalY;
        // tangent = Cross(normal, 1.0f) = (ny, -nx)
        const float tx      = ny;
        const float ty      = -nx;
        const float friction = vc.friction;
        const float mA      = vc.invMassA;
        const float mB      = vc.invMassB;
        const float iA      = vc.invIA;
        const float iB      = vc.invIB;
        const int   pc      = vc.pointCount;

        // Tangent pass
        for (int j = 0; j < pc; ++j) {
            NativeVelocityConstraintPoint& vcp = vc.points[j];

            const float dvx = (vBx - wB * vcp.relVelBy) - (vAx - wA * vcp.relVelAy);
            const float dvy = (vBy + wB * vcp.relVelBx) - (vAy + wA * vcp.relVelAx);

            const float vt     = dvx * tx + dvy * ty - vc.tangentSpeed;
            float       lambda = vcp.tangentMass * (-vt);

            const float maxFriction = friction * vcp.normalImpulse;
            const float newImpulse  = std::fmaxf(std::fminf(vcp.tangentImpulse + lambda, maxFriction), -maxFriction);
            lambda                  = newImpulse - vcp.tangentImpulse;
            vcp.tangentImpulse      = newImpulse;

            const float Px = tx * lambda;
            const float Py = ty * lambda;

            vAx -= Px * mA;
            vAy -= Py * mA;
            wA  -= iA * cross_vv(vcp.relVelAx, vcp.relVelAy, Px, Py);
            vBx += Px * mB;
            vBy += Py * mB;
            wB  += iB * cross_vv(vcp.relVelBx, vcp.relVelBy, Px, Py);
        }

        // Normal pass
        if (pc == 1) {
            NativeVelocityConstraintPoint& vcp = vc.points[0];

            const float dvx = (vBx - wB * vcp.relVelBy) - (vAx - wA * vcp.relVelAy);
            const float dvy = (vBy + wB * vcp.relVelBx) - (vAy + wA * vcp.relVelAx);

            const float vn     = dvx * nx + dvy * ny;
            float       lambda = -vcp.normalMass * (vn - vcp.velocityBias);

            const float newImpulse = std::fmaxf(vcp.normalImpulse + lambda, 0.0f);
            lambda             = newImpulse - vcp.normalImpulse;
            vcp.normalImpulse  = newImpulse;

            const float Px = nx * lambda;
            const float Py = ny * lambda;

            vAx -= Px * mA;
            vAy -= Py * mA;
            wA  -= iA * cross_vv(vcp.relVelAx, vcp.relVelAy, Px, Py);
            vBx += Px * mB;
            vBy += Py * mB;
            wB  += iB * cross_vv(vcp.relVelBx, vcp.relVelBy, Px, Py);

        } else {
            NativeVelocityConstraintPoint& cp1 = vc.points[0];
            NativeVelocityConstraintPoint& cp2 = vc.points[1];

            const float ax = cp1.normalImpulse;
            const float ay = cp2.normalImpulse;

            const float dv1x = (vBx - wB * cp1.relVelBy) - (vAx - wA * cp1.relVelAy);
            const float dv1y = (vBy + wB * cp1.relVelBx) - (vAy + wA * cp1.relVelAx);
            const float dv2x = (vBx - wB * cp2.relVelBy) - (vAx - wA * cp2.relVelAy);
            const float dv2y = (vBy + wB * cp2.relVelBx) - (vAy + wA * cp2.relVelAx);

            float vn1 = dv1x * nx + dv1y * ny;
            float vn2 = dv2x * nx + dv2y * ny;

            float bx = vn1 - cp1.velocityBias;
            float by = vn2 - cp2.velocityBias;

            const float k11 = vc.k[0];
            const float k12 = vc.k[1];  // == K.Y && K.Z (in C#)
            const float k22 = vc.k[3];

            bx -= k11 * ax + k12 * ay;
            by -= k12 * ax + k22 * ay;

            const float* nm = vc.normalMass;

            //Pre-cache
            const float c1rAx = cp1.relVelAx, c1rAy = cp1.relVelAy;
            const float c2rAx = cp2.relVelAx, c2rAy = cp2.relVelAy;
            const float c1rBx = cp1.relVelBx, c1rBy = cp1.relVelBy;
            const float c2rBx = cp2.relVelBx, c2rBy = cp2.relVelBy;

            for (;;) {
                // Case 1: vn = 0  ->  x = -NormalMass * b'
                float xx = -(nm[0] * bx + nm[1] * by);
                float xy = -(nm[2] * bx + nm[3] * by);

                if (xx >= 0.0f && xy >= 0.0f) {
                    const float dx = xx - ax, dy = xy - ay;
                    const float P1x = nx * dx, P1y = ny * dx;
                    const float P2x = nx * dy, P2y = ny * dy;
                    _rg_apply_impulse_pair(P1x, P1y, P2x, P2y,
                                       c1rAx, c1rAy, c2rAx, c2rAy,
                                       c1rBx, c1rBy, c2rBx, c2rBy,
                                       mA, iA, mB, iB,
                                       vAx, vAy, wA, vBx, vBy, wB);
                    cp1.normalImpulse = xx;
                    cp2.normalImpulse = xy;
                    break;
                }

                // Case 2: vn1 = 0, x2 = 0
                xx   = -cp1.normalMass * bx;
                xy   = 0.0f;
                vn1  = 0.0f;
                vn2  = k12 * xx + by;

                if (xx >= 0.0f && vn2 >= 0.0f) {
                    const float dx = xx - ax, dy = xy - ay;
                    const float P1x = nx * dx, P1y = ny * dx;
                    const float P2x = nx * dy, P2y = ny * dy;
                    _rg_apply_impulse_pair(P1x, P1y, P2x, P2y,
                                       c1rAx, c1rAy, c2rAx, c2rAy,
                                       c1rBx, c1rBy, c2rBx, c2rBy,
                                       mA, iA, mB, iB,
                                       vAx, vAy, wA, vBx, vBy, wB);
                    cp1.normalImpulse = xx;
                    cp2.normalImpulse = xy;
                    break;
                }

                // Case 3: vn2 = 0, x1 = 0
                xx   = 0.0f;
                xy   = -cp2.normalMass * by;
                vn1  = k12 * xy + bx;  // K.Z * x.Y + b.X  (K.Z == k12)
                vn2  = 0.0f;

                if (xy >= 0.0f && vn1 >= 0.0f) {
                    const float dx = xx - ax, dy = xy - ay;
                    const float P1x = nx * dx, P1y = ny * dx;
                    const float P2x = nx * dy, P2y = ny * dy;
                    _rg_apply_impulse_pair(P1x, P1y, P2x, P2y,
                                       c1rAx, c1rAy, c2rAx, c2rAy,
                                       c1rBx, c1rBy, c2rBx, c2rBy,
                                       mA, iA, mB, iB,
                                       vAx, vAy, wA, vBx, vBy, wB);
                    cp1.normalImpulse = xx;
                    cp2.normalImpulse = xy;
                    break;
                }

                // Case 4: x1 = 0, x2 = 0
                xx   = 0.0f;
                xy   = 0.0f;
                vn1  = bx;
                vn2  = by;

                if (vn1 >= 0.0f && vn2 >= 0.0f) {
                    const float dx = xx - ax, dy = xy - ay;
                    const float P1x = nx * dx, P1y = ny * dx;
                    const float P2x = nx * dy, P2y = ny * dy;
                    _rg_apply_impulse_pair(P1x, P1y, P2x, P2y,
                                       c1rAx, c1rAy, c2rAx, c2rAy,
                                       c1rBx, c1rBy, c2rBx, c2rBy,
                                       mA, iA, mB, iB,
                                       vAx, vAy, wA, vBx, vBy, wB);
                    cp1.normalImpulse = xx;
                    cp2.normalImpulse = xy;
                    break;
                }

                // No valid solution found - Box2D says this is fine, just bail
                break;
            }
        }
    }
}

#ifdef __cplusplus
}
#endif
