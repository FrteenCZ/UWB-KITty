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
      I(dim * 2, dim * 2),
      last_update_time_ms_(0)
{

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
    // Set the initial position from the first measurement.
    // Assume initial velocity is zero.
    for (int i = 0; i < dim_measure_; ++i)
    {
        X[i][0] = initial_measurement[i][0];
    }
    is_initialized_ = true;
}

void KalmanFilter::predict(unsigned long current_time)
{
    if (!is_initialized_)
        return;

    float dt = (last_update_time_ms_ == 0) ? 0.01f : (current_time - last_update_time_ms_) / 1000.0f;
    last_update_time_ms_ = current_time;

    // --- Update the State Transition Matrix (F) with dt ---
    F.set_identity();
    for (int i = 0; i < dim_measure_; ++i)
    {
        F[i][i + dim_measure_] = dt;
    }

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

void KalmanFilter::update(Matrix measurement, const Matrix &null_space, float alpha, unsigned long current_time)
{
    // If not initialized, use this measurement to initialize the state.
    if (!is_initialized_)
    {
        init(measurement);
        return;
    }

    // predict the current state
    predict(current_time);

    // deal with the null space
    if (null_space.cols() != 0)
    {
        Matrix x = measurement; // vector from measurement to predicted state
        for (int i = 0; i < measurement.rows(); i++)
        {
            x[i][0] -= getState()[i][0];
        }
        Matrix w = null_space * null_space.transpose() * x; // x vector projected on the null space
        float w_lenght = w.norm();
        if (w_lenght != 0.0f)
        {
            measurement = measurement - w * (alpha / w_lenght);
        }
        else
        {
            measurement = measurement + null_space.getColumn(0) * alpha;
        }
    }

    Matrix Y = measurement - (H * X);             // Measurement residual
    Matrix S = H * P * H.transpose() + R;         // Residual covariance
    Matrix K = P * H.transpose() * S.inverseQR(); // Kalman gain (using regular inverse for simplicity)

    // Update state estimate and covariance
    X = X + K * Y;
    P = (I - K * H) * P;
}

Matrix KalmanFilter::getState() const
{
    return X;
}