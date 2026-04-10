#ifndef R3_H
#define R3_H

#include <math.h>

/* Вектор (точка) в R3 */
typedef struct {
    double x, y, z;
} R3;

/* Конструктор */
static inline R3 r3_create(double x, double y, double z) {
    R3 v = {x, y, z};
    return v;
}

/* Сумма векторов */
static inline R3 r3_add(R3 a, R3 b) {
    return r3_create(a.x + b.x, a.y + b.y, a.z + b.z);
}

/* Разность векторов */
static inline R3 r3_sub(R3 a, R3 b) {
    return r3_create(a.x - b.x, a.y - b.y, a.z - b.z);
}

/* Умножение на число */
static inline R3 r3_mul(R3 v, double k) {
    return r3_create(k * v.x, k * v.y, k * v.z);
}

/* Поворот вокруг оси Oz */
static inline R3 r3_rz(R3 v, double fi) {
    return r3_create(
        cos(fi) * v.x - sin(fi) * v.y,
        sin(fi) * v.x + cos(fi) * v.y,
        v.z
    );
}

/* Поворот вокруг оси Oy */
static inline R3 r3_ry(R3 v, double fi) {
    return r3_create(
        cos(fi) * v.x + sin(fi) * v.z,
        v.y,
        -sin(fi) * v.x + cos(fi) * v.z
    );
}

/* Скалярное произведение */
static inline double r3_dot(R3 a, R3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

/* Векторное произведение */
static inline R3 r3_cross(R3 a, R3 b) {
    return r3_create(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

#endif /* R3_H */
