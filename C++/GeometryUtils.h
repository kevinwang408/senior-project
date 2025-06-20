#ifndef GEOMETRY_UTILS_H
#define GEOMETRY_UTILS_H

#include <cmath>

// Computes the inner product of two 2D vectors
inline double INNER_PRODUCT(double a, double b, double c, double d) {
    return a * c + b * d;
}

// Computes the magnitude of a 2D vector
inline double mag(double a, double b) {
    return std::sqrt(a * a + b * b);
}

// Computes the cosine of the angle between two 2D vectors
inline double COS_VAL(double a, double b, double c, double d) {
    return (INNER_PRODUCT(a, b, c, d) / (mag(a, b) * mag(c, d)));
}

// Computes the perpendicular distance from point (x0, y0) to line passing through vector (vec_x, vec_y) at (pass_x, pass_y)
inline double dis(double vec_x, double vec_y, double pass_x, double pass_y, double x0, double y0) {
    double c = vec_y * pass_x - vec_x * pass_y;
    double distance = (vec_y * x0 - vec_x * y0 - c) / std::sqrt(vec_x * vec_x + vec_y * vec_y);
    return distance;
}

#endif // GEOMETRY_UTILS_H
