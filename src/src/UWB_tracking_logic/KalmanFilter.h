#ifndef KALMAN_FILTER_H
#define KALMAN_FILTER_H

#include "matrix.h"

class KalmanFilter
{
public:
    /**
     * @brief Kalman filter constructor.
     * @param dim The number of spatial dimensions (e.g., 2 for 2D tracking [x, y]).
     * @param process_noise Standard deviation of the process noise (acceleration).
     * @param measurement_noise Standard deviation of the measurement noise.
     */
    KalmanFilter(int dim, float process_noise, float measurement_noise);

    /**
     * @brief Initializes the filter with the first measurement.
     * @param initial_measurement The first measurement vector (positions).
     */
    void init(const Matrix& initial_measurement);

    /**
     * @brief Predicts the next state of the system.
     * @param dt The time step since the last prediction.
     */
    void predict(float dt);

    /**
     * @brief Updates the state of the system based on a new measurement.
     * @param measurement The new measurement vector (positions).
     */
    void update(const Matrix& measurement);

    /**
     * @brief Gets the current estimated state.
     * @return The state vector [pos_1, ..., pos_n, vel_1, ..., vel_n].
     */
    Matrix getState() const;

private:
    bool is_initialized_;
    int dim_state_;      // Dimension of the state vector (e.g., 4 for 2D)
    int dim_measure_;    // Dimension of the measurement vector (e.g., 2 for 2D)
    float process_noise_std_; // Standard deviation for process noise

    Matrix X; // State vector
    Matrix F; // State transition matrix
    Matrix P; // State covariance matrix
    Matrix Q; // Process noise covariance matrix
    Matrix H; // Measurement matrix
    Matrix R; // Measurement noise covariance matrix
    Matrix I; // Identity matrix
};

#endif // KALMANFILTER_H
