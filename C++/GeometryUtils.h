// GeometryUtils.h
// ===========================================================================
// This header provides fundamental geometric functions for 2D vector math,
// useful for billiards path planning and collision detection.
// Functions include vector magnitude, dot product, angle cosine, and
// perpendicular distance from a point to a line.
// ===========================================================================

#ifndef GEOMETRY_UTILS_H
#define GEOMETRY_UTILS_H

#include <cmath>

// ---------------------------------------------------------------------------
// Computes the inner (dot) product of two 2D vectors:
// Vector A = (a, b), Vector B = (c, d)
// Inner product = a*c + b*d
// This is used to calculate projection, angle cosine, and alignment.
// ---------------------------------------------------------------------------
inline double INNER_PRODUCT(double a, double b, double c, double d) {
    return a * c + b * d;
}

// ---------------------------------------------------------------------------
// Computes the magnitude (Euclidean norm) of a 2D vector:
// Vector V = (a, b)
// Magnitude = sqrt(a^2 + b^2)
// This represents the vector's length or distance in 2D space.
// ---------------------------------------------------------------------------
inline double mag(double a, double b) {
    return std::sqrt(a * a + b * b);
}

// ---------------------------------------------------------------------------
// Computes the cosine of the angle between two vectors:
// Cos(theta) = (A • B) / (|A| * |B|)
// Where A = (a, b) and B = (c, d)
// This is often used to measure alignment between vectors:
// - 1.0 means same direction
// - 0.0 means perpendicular
// - -1.0 means opposite direction
// ---------------------------------------------------------------------------
inline double COS_VAL(double a, double b, double c, double d) {
    return (INNER_PRODUCT(a, b, c, d) / (mag(a, b) * mag(c, d)));
}

// ---------------------------------------------------------------------------
// Calculates the perpendicular distance from a point (x0, y0)
// to a line defined by a direction vector (vec_x, vec_y) and base point (pass_x, pass_y).
//
// The formula derives from the point-to-line distance:
// Distance = |vec_y * x0 - vec_x * y0 - (vec_y * pass_x - vec_x * pass_y)| / sqrt(vec_x^2 + vec_y^2)
//
// This function is crucial for checking whether any ball obstructs a direct path.
// ---------------------------------------------------------------------------
inline double dis(double vec_x, double vec_y, double pass_x, double pass_y, double x0, double y0) {
    double c = vec_y * pass_x - vec_x * pass_y;
    double numerator = vec_y * x0 - vec_x * y0 - c;
    double denominator = std::sqrt(vec_x * vec_x + vec_y * vec_y);
    double distance = numerator / denominator;
    return distance;
}

#endif // GEOMETRY_UTILS_H
