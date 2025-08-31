#include "trilateration.h"

/**
 * @brief Initialize the trilateration algorithm.
 *
 * @param numOfDimensions The number of dimensions (2D or 3D)
 */
trilateration::trilateration(int numOfDimensions)
    : numOfDimensions(numOfDimensions)
{
    // Initialize the Kalman filter with the specified number of dimensions
    kf = KalmanFilter(numOfDimensions);

    // Initialize the buffer index and count
    bufferIndex = 0;
    count = 0;
}

/**
 * @brief Update the trilateration algorithm with a new data point.
 *
 * @param point The data point containing coordinates and distance
 */
void trilateration::updateSinglePoint(const DataPoint &point)
{
    // Store the data point in the buffer
    buffer[bufferIndex] = point;
    bufferIndex = (bufferIndex + 1) % BUFFER_SIZE;
    if (count < BUFFER_SIZE)
        count++;

    // Check if we have enough points to compute the least squares solution
    if (count < (numOfDimensions + 1)) // At least numOfDimensions + 1 points are needed
    {
        Serial.println("Not enough points to compute the least squares solution.");
        return;
    }

    // Create matrices for the anchor points and distances
    Matrix cords(count, numOfDimensions);
    Matrix distances(count, 1);

    // Copy the data points to the matrices
    for (int i = 0; i < count; ++i)
    {
        for (int j = 0; j < numOfDimensions; ++j)
        {
            cords[i][j] = (j == 0) ? buffer[i].x : (j == 1) ? buffer[i].y
                                                            : buffer[i].z;
        }
        distances[i][0] = buffer[i].d;
    }

    update(cords, distances);
}

/**
 * @brief Update the trilateration algorithm with a set of anchor points and distances.
 *
 * @param cords The coordinates of the anchor points (numOfPoints x numOfDimensions)
 * @param distances The distances to the target from each anchor point (numOfPoints x 1)
 */
void trilateration::update(const Matrix &cords, const Matrix &distances)
{
    // Check if the number of dimensions matches the input
    if (cords.cols() != numOfDimensions)
    {
        Serial.printf("Error: Expected %d dimensions, but got %d.\n", numOfDimensions, cords.cols());
        return;
    }
    // Check if the number of distances matches the number of points
    if (distances.rows() != cords.rows())
    {
        Serial.printf("Error: Number of distances (%d) does not match number of points (%d).\n", distances.rows(), cords.rows());
        return;
    }
    // Check if we have enough points to compute the least squares solution
    if (cords.rows() < (numOfDimensions + 1))
    {
        Serial.println("Not enough points to compute the least squares solution.");
        return;
    }

    // Compute the centroid of the anchor points
    Matrix centroid(1, cords.cols()); // Centroid of the anchor points
    for (int j = 0; j < cords.cols(); ++j)
    {
        float sum = 0;
        for (int i = 0; i < cords.rows(); ++i)
        {
            sum += cords[i][j];
        }
        centroid[0][j] = sum / cords.rows();
    }

    // Center the coordinates
    Matrix centeredCords(cords.matrix); // Cords centered around the origin
    for (int i = 0; i < cords.rows(); ++i)
    {
        for (int j = 0; j < cords.cols(); ++j)
        {
            centeredCords[i][j] -= centroid[0][j];
        }
    }

    // Compute the covariance matrix of the centered coordinates
    Matrix cov = covariance(centeredCords); // Covariance matrix of the centered coordinates

    // Compute the eigenvalues and eigenvectors of the covariance matrix
    Matrix eigenvalues; // Eigenvalues of the covariance matrix
    Matrix V;           // Eigenvectors of the covariance matrix
    std::tie(eigenvalues, V) = cov.eigenJacobi();
    Serial.println("Eigenvalues:");
    eigenvalues.print();
    Serial.println("Eigenvectors:");
    V.print();

    // Transform the coordinates to the eigenvector space
    Matrix transformedCords = (V.transpose() * centeredCords.transpose()).transpose(); // Coordinates transformed to the eigenvector space
    Serial.println("Transformed Coordinates:");
    transformedCords.print();

    // --- Dimensionality reduction ---
    // Find how many dimensions to keep based on the eigenvalues
    int dimensionsToKeep = 1;
    for (int i = 0; i < eigenvalues.rows(); ++i)
    {
        float flatness = eigenvalues[i][0] / eigenvalues[0][0]; // Flatness ratio
        Serial.printf("Flatness of dimension %d: %.6f\n", i, flatness);
        if (flatness < 0.01) // Threshold for negligible flatness
        {
            break;
        }
        
        dimensionsToKeep = i + 1; // Increment the count of dimensions to keep
    }
    Serial.printf("Dimensions to keep: %d/%d\n", dimensionsToKeep, eigenvalues.rows());

    // Reduce the dimensionality of the transformed coordinates
    Matrix reducedCords(transformedCords.rows(), dimensionsToKeep);
    for (int i = 0; i < transformedCords.rows(); ++i)
    {
        for (int j = 0; j < dimensionsToKeep; ++j)
        {
            reducedCords[i][j] = transformedCords[i][j];
        }
    }
    Serial.println("Reduced Coordinates:");
    reducedCords.print();

    // --- Trilateration ---
    // Compute the linear equations
    std::pair<Matrix, Matrix> equations = computeEquations(reducedCords, distances);
    Matrix A = equations.first;  // Coefficient matrix
    Matrix b = equations.second; // Right-hand side vector
    Serial.println("Coefficient Matrix A:");
    A.print();
    Serial.println("Right-hand Side Vector b:");
    b.print();

    // Solve the linear equations using least squares
    Matrix x = solveLeastSquares(A, b); // Least squares solution
    Serial.println("Transformed Least Squares Solution x:");
    x.print();

    // --- Transform back to original space ---
    // Add the missing dimensions to the solution
    Matrix lsSolution(x.rows(), numOfDimensions);
    for (int i = 0; i < dimensionsToKeep; i++)
    {
        lsSolution[0][i] = x[0][i]; // Copy the solution for the reduced dimensions
    }

    // Transform the solution back to the original coordinate space
    Matrix originalSolution = (V * lsSolution.transpose()).transpose(); // Transform back to the original space
    Serial.println("Original Space Solution:");
    originalSolution.print();

    // Add the centroid to the solution
    originalSolution = originalSolution + centroid; // Add the centroid to the solution
    Serial.println("Final Point:");
    originalSolution.print();

    // Print the solution in JSON format
    Serial.print("Trilateration Solution:");
    originalSolution.printJSON();

    // Update the Kalman filter with the new solution
    kf.update(originalSolution);

    // Print the current state of the Kalman filter
    Serial.print("Kalman Filter State JSON:");
    getState().transpose().printJSON();
}

/**
 * @brief Get the current state of the Kalman filter.
 *
 * @return Matrix The current state of the Kalman filter.
 */
Matrix trilateration::getState() const
{
    return kf.getState();
}

/**
 * @brief Print the contents of the buffer.
 */
void trilateration::printBuffer() const
{
    Serial.println("Buffer contents:");
    for (int i = 0; i < count; ++i)
    {
        Serial.printf("Point %d: x=%.2f, y=%.2f, z=%.2f, d=%.2f\n", i, buffer[i].x, buffer[i].y, buffer[i].z, buffer[i].d);
    }
}