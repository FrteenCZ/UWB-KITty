#ifndef LEASTSQUARE_H
#define LEASTSQUARE_H

#include <vector>
#include "matrix.h"

struct Plane {
    float a, b, c, d; // Plane equation: ax + by + cz + d = 0
};

Matrix computeCentroid(const Matrix &cords);
std::pair<Matrix, Matrix> computeEquations(const Matrix &cords, const Matrix &distances);

std::tuple<Matrix, Matrix, Matrix> svd(const Matrix &A);
bool isCoplanar(const Matrix Sigma, float threshold = 1e-5);
bool isCollinear(const Matrix &Sigma, float threshold = 1e-5);
Plane findPlane(const Matrix &V, const Matrix &Centroid);
Matrix projectPointsOntoPlane(const Matrix &points, const Plane &plane);
Matrix convert3DTo2D(const Matrix &points, const Matrix &planeU, const Matrix &planeV);
Matrix reconstruct3D(const Matrix &lsSolution2D, const Matrix &planeU, const Matrix &planeV);

Matrix solveLeastSquares(const Matrix &A, const Matrix &b);

#endif // LEASTSQUARE_H