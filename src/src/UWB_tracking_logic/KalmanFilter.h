#ifndef KALMAN_FILTER_H
#define KALMAN_FILTER_H

#include "matrix.h"

/**
 * @brief Kalman filter class.
 */
class KalmanFilter
{
public:
    KalmanFilter(int numvberOfDimensions = 3);
    void predict(float dt);
    void update(const Matrix &measurement);
    Matrix getState() const;
    void adjustKalmanNoise();

private:
    int numOfDimensions; // Number of dimensions (2D or 3D)
    Matrix X;            // State vector [x, y, z, vx, vy, vz]
    Matrix F;            // State transition matrix
    Matrix P;            // Covariance matrix
    Matrix Q;            // Process noise covariance
    Matrix H;            // Measurement matrix
    Matrix R;            // Measurement noise covariance
    Matrix I;            // Identity matrix
    float currentQScale; // Current process noise scale
};

#endif // KALMANFILTER_H
