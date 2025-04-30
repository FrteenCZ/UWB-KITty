#include "leastSquare.h"

/**
 * @brief Compute the centroid of a set of coordinates.
 *
 * @param cords Matrix of coordinates (rows x cols)
 * @return Matrix The centroid of the coordinates
 */
Matrix computeCentroid(const Matrix &cords)
{
    int rows = cords.rows();
    int cols = cords.cols();

    // Check if the matrix is empty
    if (rows == 0 || cols == 0)
    {
        Serial.println("Error computeCentroid: Empty matrix.");
        return Matrix(1, cols);
    }

    // Compute the centroid
    Matrix centroid(1, cols);
    for (int j = 0; j < cols; ++j)
    {
        float sum = 0;
        for (int i = 0; i < rows; ++i)
        {
            sum += cords[i][j];
        }
        centroid[0][j] = sum / rows;
    }

    return centroid;
}

/**
 * @brief Compute the system of equations for the least squares problem.
 *
 * @param cords Matrix of anchor point coordinates (rows x cols)
 * @param distances Matrix of distances from the anchor points (rows x 1)
 * @return std::pair<Matrix, Matrix> (A, b) where A is the matrix of coefficients and b is the right-hand side vector
 */
std::pair<Matrix, Matrix> computeEquations(const Matrix &cords, const Matrix &distances)
{
    int rows = cords.matrix.size();    // Number of anchor points
    int cols = cords.matrix[0].size(); // Number of spatial dimensions (2 for XY, 3 for XYZ)

    // Create matrix A (rows-1 x cols) for the system of equations
    Matrix A(rows - 1, cols);
    for (int i = 1; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            A[i - 1][j] = cords[i][j] - cords[0][j];
        }
    }

    // Create vector b (rows-1 x 1) for the right-hand side of the equations
    Matrix b(rows - 1, 1);
    for (int i = 1; i < rows; ++i)
    {
        b[i - 1][0] = distances[0][0] * distances[0][0] - distances[i][0] * distances[i][0];
        for (int j = 0; j < cols; j++)
        {
            b[i - 1][0] += (cords[i][j] * cords[i][j]) - (cords[0][j] * cords[0][j]);
        }
        b[i - 1][0] /= 2.0;
    }

    return std::make_pair(A, b);
}

/**
 * @brief Compute Singular Value Decomposition (SVD) using Jacobi method.
 *
 * @param A Input matrix (m x n)
 * @return std::tuple<Matrix, Matrix, Matrix> (U, Sigma, V^T)
 */
std::tuple<Matrix, Matrix, Matrix> svd(const Matrix &A)
{
    int m = A.rows();
    int n = A.cols();

    // 1. Compute A^T * A
    Matrix At = A.transpose();
    Matrix AtA = At * A; // Use your Matrix multiplication

    // 2. Eigen-decomposition of A^T * A  (Using a power iteration-like method)
    Matrix V(n, n);
    Matrix Sigma(m, n); // Sigma is m x n
    std::vector<float> singularValues(n);

    // Initialize V to identity matrix
    V.set_identity();

    // Power iteration parameters
    int numIterations = 100;
    float tolerance = 1e-6f;

    for (int i = 0; i < n; ++i)
    {
        Matrix vk = V.getColumn(i); // Start with i-th column of V
        for (int iter = 0; iter < numIterations; ++iter)
        {
            
            Matrix vk1 = AtA * vk; // AtA * vk
            float norm_vk1 = vk1.norm();
            if (norm_vk1 < 1e-6)
            {
                // handle the zero vector.
                break;
            }
            vk1 = vk1 * (1.0f / norm_vk1); // Normalize
            if ((vk1 - vk).norm() < tolerance)
                break;
            vk = vk1;
        }
        // Store the eigenvector as a column in V
        V.setColumn(i, vk);
        singularValues[i] = std::sqrt((vk.transpose() * AtA * vk)[0][0]); // calculate the singular value.
        Sigma[i][i] = singularValues[i];

        // Deflate AtA  (remove the contribution of the found eigenpair)
        AtA = AtA - vk * vk.transpose() * singularValues[i] * singularValues[i];
    }

    Serial.println("Eigen-decomposition complete.");
    // Sort singular values and corresponding vectors in descending order.
    std::vector<std::pair<float, Matrix>> sv_and_v;
    for (int i = 0; i < n; ++i)
    {
        sv_and_v.push_back(std::make_pair(singularValues[i], V.getColumn(i)));
    }
    std::sort(sv_and_v.begin(), sv_and_v.end(), [](const std::pair<float, Matrix> &a, const std::pair<float, Matrix> &b)
              { return a.first > b.first; });
    // construct V and Sigma from the sorted pairs.
    for (int i = 0; i < n; ++i)
    {
        singularValues[i] = sv_and_v[i].first;
        V.setColumn(i, sv_and_v[i].second);
        Sigma[i][i] = singularValues[i];
    }
    // 3. Compute U
    Matrix U(m, m);
    // Initialize U
    Matrix AAt = A * At;
    for (int i = 0; i < m; ++i)
    {
        U[i][i] = 1.0f;
    }
    for (int i = 0; i < n; ++i)
    {
        Matrix ui = (A * V.getColumn(i)) * (1.0f / singularValues[i]);
        U.setColumn(i, ui);
    }
    if (m > n)
    {
        // Compute an orthonormal basis for the null space of A^T
        Matrix nullspace_basis(m, m - n);
        Matrix temp(m, m);
        temp.set_identity();
        for (int i = 0; i < m - n; ++i)
        {
            nullspace_basis.setColumn(i, temp.getColumn(n + i));
        }
        Matrix Q, R;
        std::tie(Q, R) = nullspace_basis.qrDecomposition();
        for (int i = 0; i < m - n; ++i)
        {
            U.setColumn(n + i, Q.getColumn(i));
        }
    }
    return std::make_tuple(U, Sigma, V);
}

/**
 * @brief Check if the points are coplanar based on the singular values.
 *
 * @param Sigma The diagonal matrix of singular values
 * @param threshold The threshold for coplanarity
 * @return true If the points are coplanar
 * @return false If the points are not coplanar
 */
bool isCoplanar(const Matrix Sigma, float threshold)
{
    // The smallest singular value is in the last diagonal element of Sigma
    float sigmaMin = Sigma[Sigma.cols() - 1][Sigma.cols() - 1];

    // If sigmaMin is very small, points are coplanar
    return fabs(sigmaMin) < threshold;
}

/**
 * @brief Check if all given 3D points are collinear using cross product.
 *
 * @param points A matrix of 3D points (each row is a point [x, y, z]).
 * @return true if collinear, false otherwise.
 */
bool isCollinear(const Matrix &points, float threshold)
{
    if (points.rows() < 3)
        return true; // Less than 3 points are always collinear

    // Pick the first point to compare with all others
    float x1 = points[0][0], y1 = points[0][1], z1 = points[0][2];

    // Create a reference vector using the first two points
    float x2 = points[1][0], y2 = points[1][1], z2 = points[1][2];
    float Ax = x2 - x1, Ay = y2 - y1, Az = z2 - z1;

    // Check all subsequent points
    for (int i = 2; i < points.rows(); ++i)
    {
        float x3 = points[i][0], y3 = points[i][1], z3 = points[i][2];

        // Create vector B from the first point to the current point
        float Bx = x3 - x1, By = y3 - y1, Bz = z3 - z1;

        // Compute the cross product of A and B
        float crossX = Ay * Bz - Az * By;
        float crossY = Az * Bx - Ax * Bz;
        float crossZ = Ax * By - Ay * Bx;

        float crossMagnitude = sqrt(crossX * crossX + crossY * crossY + crossZ * crossZ);

        // If the cross product is not close to zero, points are not collinear
        if (crossMagnitude > threshold)

        {
            return false; // Points are not collinear
        }
    }
    return true; // All points are collinear
}

/**
 * @brief Find the plane equation from the SVD results.
 *
 * @param V The matrix V from SVD
 * @param Centroid The centroid of the points
 * @return Plane The plane equation coefficients
 * @note The plane is defined as ax + by + cz + d = 0
 */
Plane findPlane(const Matrix &V, const Matrix &Centroid)
{
    Plane plane;
    plane.a = V[0][2];                                                                           // Normal x
    plane.b = V[1][2];                                                                           // Normal y
    plane.c = V[2][2];                                                                           // Normal z
    plane.d = -(plane.a * Centroid[0][0] + plane.b * Centroid[0][1] + plane.c * Centroid[0][2]); // Plane equation: ax + by + cz + d = 0

    return plane;
}

/**
 * @brief Project points onto a plane defined by the plane equation.
 *
 * @param points The points to project (rows x 3)
 * @param plane The plane equation coefficients
 * @return Matrix The projected points
 */
Matrix projectPointsOntoPlane(const Matrix &points, const Plane &plane)
{
    if (points.rows() == 0 || points.cols() != 3)
    {
        Serial.println("Error: Invalid dimensions for points.");
        return points;
    }

    float a = plane.a;
    float b = plane.b;
    float c = plane.c;
    float d = plane.d;

    Matrix projectedPoints(points.rows(), points.cols());

    for (int i = 0; i < points.rows(); ++i)
    {
        float x = points[i][0];
        float y = points[i][1];
        float z = points[i][2];

        // Calculate the distance from the point to the plane
        float lambda = (a * x + b * y + c * z + d) / (a * a + b * b + c * c);

        // Project the point onto the plane
        projectedPoints[i][0] = x - lambda * a;
        projectedPoints[i][1] = y - lambda * b;
        projectedPoints[i][2] = z - lambda * c;
    }

    return projectedPoints;
}

/**
 * @brief Convert a 3D point on a plane into 2D coordinates using plane basis vectors.
 *
 * @param point The 3D point (Matrix 1x3)
 * @param planeU First basis vector of the plane (Matrix 1x3)
 * @param planeV Second basis vector of the plane (Matrix 1x3)
 * @return Matrix The 2D coordinates (Matrix 1x2)
 */
Matrix convert3DTo2D(const Matrix &points, const Matrix &planeU, const Matrix &planeV)
{
    if (points.cols() != 3 || planeU.cols() != 3 || planeV.cols() != 3)
    {
        Serial.println("Error: Matrices must have 3 columns for 3D points.");
        return Matrix(points.rows(), 2);
    }

    Matrix result(points.rows(), 2);
    for (int i = 0; i < points.rows(); ++i)
    {
        // Project the 3D point onto the plane using the basis vectors
        result[i][0] = points[i][0] * planeU[0][0] + points[i][1] * planeU[0][1] + points[i][2] * planeU[0][2];
        result[i][1] = points[i][0] * planeV[0][0] + points[i][1] * planeV[0][1] + points[i][2] * planeV[0][2];
    }

    return result;
}

/**
 * @brief Reconstruct a 3D point from its 2D coordinates using the plane basis vectors.
 *
 * @param lsSolution2D The 2D coordinates (Matrix 1x2)
 * @param planeU First basis vector of the plane (Matrix 1x3)
 * @param planeV Second basis vector of the plane (Matrix 1x3)
 * @return Matrix The reconstructed 3D point (Matrix 1x3)
 */
Matrix reconstruct3D(const Matrix &lsSolution2D, const Matrix &planeU, const Matrix &planeV)
{
    // Check if the input matrices have the correct dimensions
    if (lsSolution2D.cols() != 2)
    {
        Serial.println("Error reconstruct3D: lsSolution2D must have 2 rows.");
        return Matrix(1, 3);
    }
    if (planeU.cols() != 3 || planeV.cols() != 3)
    {
        Serial.println("Error reconstruct3D: planeU and planeV must have 3 columns.");
        return Matrix(1, 3);
    }

    // Reconstruct the 3D point using the basis vectors
    Matrix result(1, 3);
    result[0][0] = lsSolution2D[0][0] * planeU[0][0] + lsSolution2D[0][1] * planeV[0][0];
    result[0][1] = lsSolution2D[0][0] * planeU[0][1] + lsSolution2D[0][1] * planeV[0][1];
    result[0][2] = lsSolution2D[0][0] * planeU[0][2] + lsSolution2D[0][1] * planeV[0][2];

    return result;
}

/**
 * @brief Solve the Least Squares using QR Decomposition
 *
 * @param A The matrix A
 * @param b The matrix b
 * @return Matrix The solution x
 */
Matrix solveLeastSquares(const Matrix &A, const Matrix &b)
{
    if (A.rows() == 0 || A.cols() == 0 || b.rows() == 0 || b.cols() != 1)
    {
        Serial.println("Error solveLeastSquares: Invalid dimensions for A or b.");
        return Matrix(1, A.cols());
    }
    if (A.rows() < A.cols())
    {
        Serial.println("Error solveLeastSquares: More variables than equations.");
        return Matrix(1, A.cols());
    }
    if (A.rows() != b.rows())
    {
        Serial.println("Error solveLeastSquares: Incompatible dimensions for A and b.");
        return Matrix(1, A.cols());
    }

    // Perform QR decomposition
    std::pair<Matrix, Matrix> qrResult = A.qrDecomposition();
    Matrix Q = qrResult.first;
    Matrix R = qrResult.second;

    // Compute Q^T * b
    Matrix Qtb = Q.transpose() * b;

    // Solve for x using inverse of R
    Matrix result = R.inverseQR() * Qtb;

    return result.transpose();
}