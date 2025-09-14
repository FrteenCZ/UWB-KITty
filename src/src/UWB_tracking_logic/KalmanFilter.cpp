#include "KalmanFilter.h"
#include <cmath>

KalmanFilter::KalmanFilter(int dim, float process_noise, float measurement_noise)
    : is_initialized_(false),
      dim_state_(dim * 2),
      dim_measure_(dim),
      process_noise_std_(process_noise),
      X(dim * 2, 1),
      F{dim * 2, dim * 2},
      P(dim * 2, dim * 2),
      Q(dim * 2, dim * 2),
      H(dim, dim * 2),
      R(dim, dim),
      I(dim * 2, dim * 2)
{

    // --- FIX 1: Correctly define the Measurement Matrix (H) ---
    // H maps the state vector to the measurement space.
    // We only measure position, so H should be [1, 0, 0, 0; 0, 1, 0, 0] for 2D.
    // H.set_zero();
    for (int i = 0; i < dim_measure_; ++i)
    {
        H[i][i] = 1.0f;
    }

    // --- Initialize other matrices ---
    R.set_identity(pow(measurement_noise, 2)); // R = sigma_measure^2
    P.set_identity(500.0f);                    // High initial uncertainty in our state estimate
    I.set_identity();
}

void KalmanFilter::init(const Matrix &initial_measurement)
{
    // --- FIX 2: Explicit Initialization Step ---
    // Set the initial position from the first measurement.
    // Assume initial velocity is zero.
    for (int i = 0; i < dim_measure_; ++i)
    {
        X[i][0] = initial_measurement[i][0];
    }
    is_initialized_ = true;
}

void KalmanFilter::predict(float dt)
{
    if (!is_initialized_)
        return;

    // --- Update the State Transition Matrix (F) with dt ---
    F.set_identity();
    for (int i = 0; i < dim_measure_; ++i)
    {
        F[i][i + dim_measure_] = dt;
    }

    // --- FIX 3: Realistic Process Noise Matrix (Q) ---
    // This Q models random acceleration, which affects both position and velocity.
    float dt2 = dt * dt;
    float dt3 = dt2 * dt;
    float dt4 = dt3 * dt;
    float noise_sq = pow(process_noise_std_, 2);

    for (int i = 0; i < dim_measure_; ++i)
    {
        int pos = i;
        int vel = i + dim_measure_;
        Q[pos][pos] = dt4 / 4.0f * noise_sq;
        Q[pos][vel] = dt3 / 2.0f * noise_sq;
        Q[vel][pos] = dt3 / 2.0f * noise_sq;
        Q[vel][vel] = dt2 * noise_sq;
    }

    // Predict the next state
    X = F * X;
    P = F * P * F.transpose() + Q;
}

void KalmanFilter::update(const Matrix &measurement)
{
    // If not initialized, use this measurement to initialize the state.
    if (!is_initialized_)
    {
        init(measurement);
        return;
    }

    // --- FIX 4: Correct Measurement Residual (Y) Calculation ---
    // The original code had an incorrect transpose on the measurement.
    Matrix Y = measurement - (H * X); // Measurement residual
    Matrix S = H * P * H.transpose() + R; // Residual covariance
    Matrix K = P * H.transpose() * S.inverseQR(); // Kalman gain (using regular inverse for simplicity)

    // Update state estimate and covariance
    X = X + K * Y;
    P = (I - K * H) * P;
}

Matrix KalmanFilter::getState() const
{
    return X;
}