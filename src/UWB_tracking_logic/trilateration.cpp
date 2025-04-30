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
 * @param point The new data point (x, y, z, d)
 */
void trilateration::update(const DataPoint &point)
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

    // Check if the points are collinear (2D) or coplanar (3D)
    if (isCollinear(cords))
    {
        Serial.println("Warning: The points are collinear. Ignoring the update.");
        return;
    }
    else if (numOfDimensions == 3 && isCoplanar(cords))
    {
        Serial.println("Warning: The points are coplanar. Assuming target is on the plane.");
    }

    // Compute the centroid of the anchor points
    Matrix centroid = computeCentroid(cords);
    Serial.println("Centroid:");
    centroid.print();

    // Center the coordinates
    Matrix centeredCords(cords.matrix);
    for (int i = 0; i < cords.rows(); ++i)
    {
        for (int j = 0; j < cords.cols(); ++j)
        {
            centeredCords[i][j] -= centroid[0][j];
        }
    }
    Serial.println("Centered Cords:");
    centeredCords.print();

    // Compute the SVD of the centered coordinates
    std::tuple<Matrix, Matrix, Matrix> svdResult = svd(centeredCords);
    Matrix U = std::get<0>(svdResult);
    Matrix Sigma = std::get<1>(svdResult);
    Matrix V = std::get<2>(svdResult);
    Serial.println("U:");
    U.print();
    Serial.println("Sigma:");
    Sigma.print();
    Serial.println("V:");
    V.print();

    // Compute the linear equations
    std::pair<Matrix, Matrix> equations = computeEquations(centeredCords, distances);

    // Handle coplanar points in 3D
    if (numOfDimensions == 3 && isCoplanar(Sigma))
    {
        // Find the plane equation
        Plane plane = findPlane(V, centroid);
        Serial.printf("Plane equation: %.2fx + %.2fy + %.2fz + %.2f = 0\n", plane.a, plane.b, plane.c, plane.d);

        // Project the points onto the plane
        Matrix projectedPoints = projectPointsOntoPlane(centeredCords, plane);
        Serial.println("Projected Points:");
        projectedPoints.print();

        // Convert the 3D points to 2D coordinates
        Matrix planeU = V.getColumn(0).transpose();
        Matrix planeV = V.getColumn(1).transpose();
        planeU *= (1.0 / planeU.norm());
        planeV *= (1.0 / planeV.norm());
        Serial.println("Vector U:");
        planeU.print();
        Serial.println("Vector V:");
        planeV.print();

        // Convert the 3D points to 2D coordinates
        Matrix projected2D = convert3DTo2D(projectedPoints, planeU, planeV);
        Serial.println("Projected 2D Points:");
        projected2D.print();

        // Compute the linear equations
        equations = computeEquations(projected2D, distances);
    }

    Matrix A = equations.first;
    Matrix b = equations.second;

    // Solve the linear equations
    Matrix x = solveLeastSquares(A, b);
    Serial.println("LS Solution:");
    x.print();

    // Convert the solution back to 3D coordinates, if necessary
    if (numOfDimensions == 3 && x.cols() == 2)
    {
        Matrix lsSolution2D = x;
        Matrix planeU = V.getColumn(0).transpose();
        Matrix planeV = V.getColumn(1).transpose();
        planeU *= (1.0 / planeU.norm());
        planeV *= (1.0 / planeV.norm());
        x = reconstruct3D(lsSolution2D, planeU, planeV);
        Serial.println("Reconstructed 3D Point:");
        x.print();
    }
    x = x + centroid; // Add the centroid to the solution

    Serial.println("Final Point:");
    x.print();

    // Update the Kalman filter with the new solution
    kf.update(x);
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