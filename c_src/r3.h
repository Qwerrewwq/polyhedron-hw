#ifndef R3_H
#define R3_H

#include <math.h>

/* Vector (point) in R3 */
typedef struct {
    double x, y, z;
} R3;

/* Constructor */
static inline R3 r3_create(double x, double y, double z) {
    R3 v = {x, y, z};
    return v;
}

/* Vector addition */
static inline R3 r3_add(R3 a, R3 b) {
    return r3_create(a.x + b.x, a.y + b.y, a.z + b.z);
}

/* Vector subtraction */
static inline R3 r3_sub(R3 a, R3 b) {
    return r3_create(a.x - b.x, a.y - b.y, a.z - b.z);
}

/* Scalar multiplication */
static inline R3 r3_mul(R3 v, double k) {
    return r3_create(k * v.x, k * v.y, k * v.z);
}

/* Rotation around Oz axis */
static inline R3 r3_rz(R3 v, double fi) {
    double c = cos(fi), s = sin(fi);
    return r3_create(c * v.x - s * v.y, s * v.x + c * v.y, v.z);
}

/* Rotation around Oy axis */
static inline R3 r3_ry(R3 v, double fi) {
    double c = cos(fi), s = sin(fi);
    return r3_create(c * v.x + s * v.z, v.y, -s * v.x + c * v.z);
}

/* Dot product */
static inline double r3_dot(R3 a, R3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

/* Cross product */
static inline R3 r3_cross(R3 a, R3 b) {
    return r3_create(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

#endif /* R3_H */
