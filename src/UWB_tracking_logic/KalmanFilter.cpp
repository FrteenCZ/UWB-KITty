#include "KalmanFilter.h"

/**
 * @brief Kalman filter constructor.
 *
 * @param numOfDimensions Number of dimensions (e.g., 2 for 2D, 3 for 3D)
 */
KalmanFilter::KalmanFilter(int numOfDimensions)
    : X(numOfDimensions * 2, 1),                   // State vector: [x, y, z, vx, vy, vz] for 3D
      F{numOfDimensions * 2, numOfDimensions * 2}, // State transition matrix
      P(numOfDimensions * 2, numOfDimensions * 2), // Covariance matrix
      Q(numOfDimensions * 2, numOfDimensions * 2), // Process noise covariance
      H(numOfDimensions, numOfDimensions * 2),     // Measurement matrix
      R(numOfDimensions, numOfDimensions),         // Measurement noise covariance
      I(numOfDimensions * 2, numOfDimensions * 2)  // Identity matrix
{
    this->numOfDimensions = numOfDimensions;
    
    // Initialize matrices
    F.set_identity();
    P.set_identity(10);
    Q.set_identity();
    H.set_identity();
    R.set_identity(1);
    I.set_identity();
}

/**
 * @brief Predict the next state of the system.
 *
 * @param dt Time step
 */
void KalmanFilter::predict(float dt)
{
    // Update state transition matrix (F) for dt
    for (int i = 0; i < numOfDimensions; ++i)
    {
        F[i][i + numOfDimensions] = dt;
    }

    // Predict next state
    X = F * X;

    // Predict covariance
    P = F * P * F.transpose() + Q;
}

/**
 * @brief Update the state of the system based on the measurement.
 *
 * @param measurement Measurement vector
 */
void KalmanFilter::update(const Matrix &measurement)
{
    Matrix Y = measurement.transpose() - (H * X);           // Measurement residual
    Matrix S = H * P * H.transpose() + R;       // Residual covariance
    Matrix K = P * H.transpose() * S.inverseQR(); // Kalman gain

    // Update state
    X = X + K * Y;

    // Update covariance
    P = (I - K * H) * P;
}

/**
 * @brief Get the current state of the system.
 *
 * @return Matrix State vector [x, y, z, vx, vy, vz] for 3D
 * @note The state vector contains the position and velocity in each dimension.
 */
Matrix KalmanFilter::getState() const
{
    return X; // Return position (x, y, z, vx, vy, vz)
}

/**
 * @brief Adjust the process noise covariance matrix based on the current speed.
 */
void KalmanFilter::adjustKalmanNoise()
{
    static const float Q_MIN = 0.5f;  // Minimum process noise (stationary)
    static const float Q_MAX = 20.0f; // Maximum process noise (fast movement)
    static const float SCALE_FACTOR = 10.0f;

    float speed = 0.0f;
    for (int i = 0; i < numOfDimensions; ++i)
    {
        speed += pow(X[i + numOfDimensions][0], 2); // Sum of squared velocities
    }
    speed = sqrt(speed);

    currentQScale = Q_MIN + (Q_MAX - Q_MIN) * (speed / SCALE_FACTOR);

    Q.set_identity(currentQScale);
}
