#include "trilateration.h"
#include "UWB_tracking_logic/matrix.h"

/**
 * @brief Initialize the trilateration algorithm.
 *
 * @param numOfDimensions The number of dimensions (2D or 3D)
 */
trilateration::trilateration(int numOfDimensions)
    : numOfDimensions(numOfDimensions),
      null_space(Matrix(0, 0)),
      trilatSolution(Matrix(1, numOfDimensions)),
      alpha(0.0f),
      kf(numOfDimensions, 10.0f, 1.0f) // tune this according to your measurement precision
{
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

    update(cords, distances, millis());
}

/**
 * @brief Update the trilateration algorithm with a set of anchor points and distances.
 *
 * @param cords The coordinates of the anchor points (numOfPoints x numOfDimensions)
 * @param distances The distances to the target from each anchor point (numOfPoints x 1)
 */
void trilateration::update(const Matrix &cords, const Matrix &distances, unsigned long current_time)
{
    // --- 0. Input Validation ---
    if (cords.cols() != numOfDimensions)
    {
        Serial.printf("Error: Expected %d dimensions, but got %d.\n", numOfDimensions, cords.cols());
        return;
    }
    if (distances.rows() != cords.rows())
    {
        Serial.printf("Error: Number of distances (%d) does not match number of points (%d).\n", distances.rows(), cords.rows());
        return;
    }
    if (cords.rows() == 0)
    {
        Serial.printf("Error: There are no points");
        return;
    }

    if (cords.rows() == 1)
    {
        trilatSolution = cords;
        trilatSolution[0][0] += distances[0][0];

        kf.predict(current_time);
        kf.update(trilatSolution.transpose());
        return;
    }

    // Prevent huge dt if there's a long pause
    // if (dt > 0.5f)
    // {
    //     dt = 0.5f;
    // }


    // --- 1. Analyze Full 3D Geometry via PCA ---
    // Compute the centroid of the anchor points
    Matrix centroid(1, cords.cols());
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
    Matrix centeredCords(cords.matrix);
    for (int i = 0; i < cords.rows(); ++i)
    {
        for (int j = 0; j < cords.cols(); ++j)
        {
            centeredCords[i][j] -= centroid[0][j];
        }
    }

    // Compute the covariance matrix of the centered coordinates
    Matrix cov = covariance(centeredCords);

    // Compute the eigenvalues and eigenvectors of the covariance matrix
    Matrix eigenvalues;
    Matrix eigenvectors;
    std::tie(eigenvalues, eigenvectors) = cov.eigenJacobi();
    // Serial.println("Eigenvalues:");
    // eigenvalues.print();
    // Serial.print("Eigenvectors (E):");
    // eigenvectors.print();

    // --- 2. Transform the coordinates into the Eigenvector Basis ---
    Matrix transformedCords = centeredCords * eigenvectors;
    // Serial.println("Transformed Coordinates:");
    // transformedCords.print();

    // --- 3. Dimensionality reduction ---
    // Find how many dimensions to keep based on the eigenvalues
    int dimensionsToKeep = 0;
    if (eigenvalues[0][0] != 0)
    {
        for (int i = 0; i < eigenvalues.rows(); ++i)
        {
            float flatness = eigenvalues[i][0] / eigenvalues[0][0]; // Flatness ratio
            // Serial.printf("Flatness of dimension %d: %.6f\n", i, flatness);
            if (flatness < 0.01) // Threshold for negligible flatness
            {
                break;
            }

            dimensionsToKeep = i + 1;
        }
    }
    // Serial.printf("Dimensions to keep: %d/%d\n", dimensionsToKeep, eigenvalues.rows());

    if (dimensionsToKeep != 0)
    {

        // Reduce the dimensionality of the transformed coordinates
        Matrix reducedCords(transformedCords.rows(), dimensionsToKeep);
        for (int i = 0; i < transformedCords.rows(); ++i)
        {
            for (int j = 0; j < dimensionsToKeep; ++j)
            {
                reducedCords[i][j] = transformedCords[i][j];
            }
        }
        // Serial.println("Reduced Coordinates:");
        // reducedCords.print();

        // --- 4. Trilateration ---
        // Compute the linear equations
        std::pair<Matrix, Matrix> equations = computeEquations(reducedCords, distances);
        Matrix A = equations.first;  // Coefficient matrix
        Matrix b = equations.second; // Right-hand side vector
        // Serial.println("Coefficient Matrix A:");
        // A.print();
        // Serial.println("Right-hand Side Vector b:");
        // b.print();

        // Solve the linear equations using least squares
        Matrix x = solveLeastSquares(A, b); // Least squares solution
        // Serial.println("Transformed Least Squares Solution x:");
        // x.print();

        // --- 5. Transform back to original space ---
        // Add the missing dimensions to the solution
        Matrix lsSolution(x.rows(), numOfDimensions);
        for (int i = 0; i < dimensionsToKeep; i++)
        {
            lsSolution[0][i] = x[0][i];
        }

        // Transform the solution back to the original coordinate space
        trilatSolution = (eigenvectors * lsSolution.transpose()).transpose();
        // Serial.println("Original Space Solution:");
        // trilatSolution.print();

        // Add the centroid to the solution
        trilatSolution = trilatSolution + centroid;
        // Serial.println("Final Point:");
        // trilatSolution.print();

        // --- 6. Find alpha to get the real solution inside of the null space ---
        alpha = 0.0f;
        if (numOfDimensions - dimensionsToKeep != 0)
        {
            for (int i = 0; i < cords.rows(); ++i)
            {
                float distance = (trilatSolution - Matrix({cords[i]})).norm();
                alpha += sqrt(distances[i][0] * distances[i][0] -
                              distance * distance);
            }
            alpha /= cords.rows();
        }
        // Serial.printf("Alpha: %.6f\n", alpha);
    }
    else
    {
        trilatSolution = centroid;
        for (int i = 0; i < cords.rows(); i++)
        {
            alpha += distances[i][0];
        }
        alpha /= cords.rows();
    }

    // --- 7. Gather the null space ---
    null_space = Matrix(numOfDimensions, numOfDimensions - dimensionsToKeep);
    for (int i = 0; i < numOfDimensions; ++i)
    {
        for (int j = 0; j < numOfDimensions - dimensionsToKeep; ++j)
        {
            null_space[i][j] = eigenvectors[i][numOfDimensions - 1 - j];
        }
    }

    // --- 8. Resolve ambiguity ---
    kf.predict(current_time); // predict the current state of kalman
    Matrix measurement = trilatSolution.transpose();
    if (numOfDimensions != dimensionsToKeep) { // if ambiguity exists
        Matrix starting_position = Matrix({{0, 0, 0}});
        bool has_starting_position = false;

        // 1. Define the target state (KF, Starting Point, or Origin)
        Matrix target = kf.is_initialized_ ? kf.getState() : starting_position;
        bool use_projection = kf.is_initialized_ || has_starting_position;

        if (use_projection) {
            Matrix x = measurement;
            for (int i = 0; i < measurement.rows(); i++) {
                x[i][0] -= target[i][0];
            }

            // Project delta onto the null space
            Matrix w = null_space * null_space.transpose() * x;
            float w_length = w.norm();

            if (w_length > 1e-6f) {
                // Point = Center + (Projected Vector scaled to Alpha)
                measurement = measurement - w * (alpha / w_length);
            } else {
                // If target is exactly at center, pick any valid direction
                measurement = measurement + null_space.getColumn(0) * alpha;
            }
        } else {
            // 2. Setup Mode (The N_zero Method)
            // We want to kill the axes we aren't currently defining.
            // If defining X-axis, kill Y and Z. If defining Y, kill Z.

            int numNullVectors = null_space.cols();
            int rowsToConstrain = numNullVectors - 1;

            if (rowsToConstrain > 0) {
                Matrix N_zero(rowsToConstrain, numNullVectors);

                // Construct N_zero by picking the rows of N we want to set to 0
                // e.g., if we want to define the i-th axis, we pick all other rows
                int currentRow = 0;
                int axisToDefine = (numOfDimensions - numNullVectors); // Logic for X -> Y -> Z

                for (int i = 0; i < numOfDimensions; i++) {
                    if (i == axisToDefine) continue; // Skip the axis we are currently defining
                    if (currentRow >= rowsToConstrain) break;

                    for (int j = 0; j < numNullVectors; j++) {
                        N_zero[currentRow][j] = null_space[i][j];
                    }
                    currentRow++;
                }

                // Find the "Freedom Vector" (Null space of N_zero)
                std::tie(eigenvalues, eigenvectors) = (N_zero.transpose() * N_zero).eigenJacobi();
                // Setup mode
                        //  else {
                        //     Matrix N_zero = Matrix(numOfDimensions - dimensionsToKeep - 1, numOfDimensions - dimensionsToKeep);
                        //     for (int i = 0; i < numOfDimensions - dimensionsToKeep - 1; i++) {
                        //         for (int j = 0; j < numOfDimensions - dimensionsToKeep; i++) {
                        //             N_zero[i][j] = null_space[i+1][j];
                        //         }
                        //     }
                
                        //     std::tie(eigenvalues, eigenvectors) = (N_zero.transpose() * N_zero).eigenJacobi();
                        //     Matrix null_vector = Matrix(eigenvectors.getColumn(numOfDimensions - dimensionsToKeep - 1));
                
                        //     measurement = null_space * null_vector;
                        // }
                // Jacobi usually sorts, but ensure you grab the one with the smallest eigenvalue
                Matrix null_vector = eigenvectors.getColumn(numNullVectors - 1);

                // 3. Selection Logic: Choose + or - to satisfy the "Positive Direction" guideline
                Matrix pos_candidate = null_space * (null_vector * alpha);

                // If the coordinate of the axis we are defining is negative, flip it
                if (pos_candidate[axisToDefine][0] < 0) {
                    pos_candidate = pos_candidate * -1.0f;
                }

                measurement = measurement + pos_candidate; // Center + Offset
            } else {
                // Special case: Only 1 null vector (1D ambiguity / Point Pair)
                // Just pick the one with the positive coordinate in the highest dimension
                Matrix offset = null_space.getColumn(0) * alpha;
                if (offset[numOfDimensions - 1][0] < 0) offset = offset * -1.0f;

                measurement = measurement + offset;
            }
        }
        // // Setup mode
        //  else {
        //     Matrix N_zero = Matrix(numOfDimensions - dimensionsToKeep - 1, numOfDimensions - dimensionsToKeep);
        //     for (int i = 0; i < numOfDimensions - dimensionsToKeep - 1; i++) {
        //         for (int j = 0; j < numOfDimensions - dimensionsToKeep; i++) {
        //             N_zero[i][j] = null_space[i+1][j];
        //         }
        //     }

        //     std::tie(eigenvalues, eigenvectors) = (N_zero.transpose() * N_zero).eigenJacobi();
        //     Matrix null_vector = Matrix(eigenvectors.getColumn(numOfDimensions - dimensionsToKeep - 1));

        //     measurement = null_space * null_vector;
        // }
    }

    // --- 9. Update Kalman Filter State ---
    // Serial.print("Trilateration Solution JSON:");
    // trilatSolution.print();
    kf.update(measurement);
    // Serial.print("Kalman Filter State JSON:");
    // getState().transpose().print();
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
