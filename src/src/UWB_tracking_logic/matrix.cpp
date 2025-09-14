#include "matrix.h"

/**
 * @brief Default constructor for the Matrix class.
 *
 * Initializes an empty matrix with 0 rows and 0 columns.
 */
Matrix::Matrix()
{
    matrix = std::vector<std::vector<float>>(0, std::vector<float>(0, 0));
}

/**
 * @brief Matrix constructor.
 *
 * @param row Number of rows
 * @param col Number of columns
 */
Matrix::Matrix(int row, int col)
{
    matrix = std::vector<std::vector<float>>(row, std::vector<float>(col, 0));
}

/**
 * @brief Matrix constructor with initial values.
 *
 * @param input Initial values for the matrix
 */
Matrix::Matrix(std::vector<std::vector<float>> input)
{
    matrix = input;
}

/**
 * @brief Get the number of rows in the matrix
 *
 * @return int The number of rows
 */
int Matrix::rows() const
{
    return matrix.size();
}

/**
 * @brief Get the number of columns in the matrix
 *
 * @return int The number of columns
 */
int Matrix::cols() const
{
    return matrix[0].size();
}

/**
 * @brief Overload the * operator to multiply two matrices
 *
 * @param other The other matrix
 * @return Matrix The result of the multiplication
 */
Matrix Matrix::operator*(const Matrix &other) const
{
    if (cols() != other.rows())
    {
        Serial.println("Error: Matrices have incompatible dimensions for multiplication. ");
        Serial.printf("Matrix A: %d x %d, Matrix B: %d x %d\n", rows(), cols(), other.rows(), other.cols());
        return Matrix(rows(), other.cols());
    }

    Matrix result(rows(), other.cols());

    for (int i = 0; i < rows(); ++i)
    {
        for (int j = 0; j < other.cols(); ++j)
        {
            for (int k = 0; k < cols(); ++k)
            {
                result.matrix[i][j] += matrix[i][k] * other.matrix[k][j];
            }
        }
    }
    return result;
}

/**
 * @brief Overload the *= operator to multiply a matrix by a scalar
 *
 * @param val The scalar value
 */
void Matrix::operator*=(float val)
{
    for (size_t y = 0; y < rows(); y++)
    {
        for (size_t x = 0; x < cols(); x++)
        {
            matrix[y][x] *= val;
        }
    }
}

/**
 * @brief Overload the * operator to multiply a matrix by a scalar
 *
 * @param val The scalar value
 * @return Matrix The result of the multiplication
 */
Matrix Matrix::operator*(float val) const
{
    Matrix result(rows(), cols());
    for (size_t y = 0; y < rows(); y++)
    {
        for (size_t x = 0; x < cols(); x++)
        {
            result[y][x] = matrix[y][x] * val;
        }
    }
    return result;
}

/**
 * @brief Overload [] to return a reference to a row
 *
 * @param row The row index
 * @return std::vector<float>& The row vector
 */
std::vector<float> &Matrix::operator[](int row)
{
    if (row < 0 || row >= rows())
    {
        Serial.print("Error: Row index out of bounds. ");
        Serial.printf("Requested row: %d, but matrix has %d rows.\n", row, rows());
        return Matrix(rows(), 1)[0]; // Return vector with default values as a fallback
    }

    return matrix[row];
}

/**
 * @brief Overload [] to return a reference to a row
 *
 * @param row The row index
 * @return const std::vector<float>& The row vector
 */
const std::vector<float> &Matrix::operator[](int row) const
{
    if (row < 0 || row >= rows())
    {
        Serial.print("Error: Row index out of bounds. ");
        Serial.printf("Requested row: %d, but matrix has %d rows.\n", row, rows());
        return Matrix(rows(), 1)[0]; // Return vector with default values as a fallback
    }

    return matrix[row];
}

/**
 * @brief Get a column from the matrix
 *
 * @param colIndex The index of the column to get
 * @return Matrix The column vector
 */
Matrix Matrix::getColumn(int colIndex) const
{
    if (colIndex < 0 || colIndex >= cols())
    {
        Serial.print("Error: Column index out of bounds. ");
        Serial.printf("Requested column: %d, but matrix has %d columns.\n", colIndex, cols());
        return Matrix(rows(), 1); // Return column vector with default values as a fallback
    }

    Matrix columnVector(rows(), 1); // Create a column vector (rows x 1)

    for (int i = 0; i < rows(); ++i)
    {
        columnVector[i][0] = this->matrix[i][colIndex]; // Copy column values
    }

    return columnVector;
}

/**
 * @brief Set a column in the matrix
 *
 * @param colIndex The index of the column to set
 * @param col The column vector to set
 */
void Matrix::setColumn(int colIndex, const Matrix &col)
{
    if (col.rows() != this->rows())
    {
        // Handle error: the number of rows in the column must match the number of rows in the matrix
        Serial.println("Error: Column dimensions do not match matrix dimensions.");
        return;
    }

    for (int i = 0; i < this->rows(); ++i)
    {
        this->matrix[i][colIndex] = col[i][0]; // Update the matrix with the new column values
    }
}

/**
 * @brief Overload + to add two matrices
 *
 * @param other The other matrix
 * @return Matrix The result of the addition
 */
Matrix Matrix::operator+(const Matrix &other) const
{
    if (rows() != other.rows() || cols() != other.cols())
    {
        Serial.println("Error: Matrices have incompatible dimensions for addition.");
        Serial.printf("Matrix A: %d x %d, Matrix B: %d x %d\n", rows(), cols(), other.rows(), other.cols());
        return Matrix(rows(), cols());
    }

    Matrix result(rows(), cols());

    for (int y = 0; y < rows(); ++y)
    {
        for (int x = 0; x < cols(); ++x)
        {
            result.matrix[y][x] = matrix[y][x] + other.matrix[y][x];
        }
    }
    return result;
}

/**
 * @brief Overload - to subtract two matrices
 *
 * @param other The other matrix
 * @return Matrix The result of the subtraction
 */
Matrix Matrix::operator-(const Matrix &other) const
{
    if (rows() != other.rows() || cols() != other.cols())
    {
        Serial.println("Error: Matrices have incompatible dimensions for subtraction.");
        Serial.printf("Matrix A: %d x %d, Matrix B: %d x %d\n", rows(), cols(), other.rows(), other.cols());
        return Matrix(rows(), cols());
    }

    Matrix result(rows(), cols());

    for (int y = 0; y < rows(); ++y)
    {
        for (int x = 0; x < cols(); ++x)
        {
            result.matrix[y][x] = matrix[y][x] - other.matrix[y][x];
        }
    }
    return result;
}

/**
 * @brief Print function
 */
void Matrix::print() const
{
    for (size_t i = 0; i < rows(); ++i)
    {
        for (size_t j = 0; j < cols(); ++j)
        {
            Serial.print(matrix[i][j]);
            Serial.print(" ");
        }
        Serial.println();
    }
}

/**
 * @brief Convert the matrix to a string representation
 */
String Matrix::toString() const
{
    String output = "[";
    for (size_t i = 0; i < rows(); ++i)
    {
        output += "[";
        for (size_t j = 0; j < cols(); ++j)
        {
            output += String(matrix[i][j], 6);
            if (j < cols() - 1)
            {
                output += ", ";
            }
        }
        output += "]";
        if (i < rows() - 1)
        {
            output += ", ";
        }
    }
    output += "]";

    return output;
}

/**
 * @brief Set to value
 *
 * @param val The value to set
 */
void Matrix::set_value(float val)
{
    for (size_t y = 0; y < rows(); y++)
    {
        for (size_t x = 0; x < cols(); x++)
        {
            matrix[y][x] = val;
        }
    }
}

/**
 * @brief Set the matrix to identity
 *
 * @param scale The scale of the identity matrix
 * @param size The size of the identity matrix
 * @param y The starting row
 * @param x The starting column
 */
void Matrix::set_identity(float scale, int size, int y, int x)
{
    if (size < 0)
    {
        Serial.println("Error set_identity: Size cannot be negative.");
        return;
    }

    if (y < 0 || x < 0 || y >= rows() || x >= cols())
    {
        Serial.println("Error set_identity: Starting position is out of bounds.");
        return;
    }

    if (size > std::min(rows() - y, cols() - x))
    {
        Serial.println("Error set_identity: Size too big for the space that was provided.");
        return;
    }

    if (size == 0)
    {
        size = std::min(rows() - y, cols() - x);
    }

    set_value(0);
    for (size_t i = 0; i < size; i++)
    {
        matrix[i + y][i + x] = scale;
    }
}

/**
 * @brief Compute the Euclidean norm of the matrix
 *
 * @return float The Euclidean norm
 */
float Matrix::norm() const
{
    float sum = 0.0;

    for (const auto &row : matrix)
    {
        for (float val : row)
        {
            sum += val * val; // Square each element
        }
    }

    return sqrt(sum); // Take the square root
}

/**
 * @brief Transpose the matrix
 *
 * @return Matrix The transposed matrix
 */
Matrix Matrix::transpose() const
{
    Matrix transposed(cols(), rows());

    for (size_t y = 0; y < cols(); y++)
    {
        for (size_t x = 0; x < rows(); x++)
        {
            transposed[y][x] = matrix[x][y];
        }
    }
    return transposed;
}

/**
 * @brief Compute the inverse of the matrix using Gauss-Jordan elimination
 *
 * @return Matrix The inverse of the matrix
 */
Matrix Matrix::gaussJordanInverse() const
{
    int n = rows();

    // Check if the matrix is square
    if (n != cols())
    {
        Serial.println("Error gaussJordanInverse: Matrix must be square to compute inverse.");
        return Matrix(rows(), cols());
    }

    for (int i = 0; i < n; ++i)
    {
        if (matrix[i][i] == 0.0)
        {
            Serial.println("Error gaussJordanInverse: Singular matrix detected during Gauss-Jordan elimination.");
            return Matrix(n, n);
        }
    }

    // Create an augmented matrix [A | I]
    Matrix augmented(n, n * 2);
    for (int y = 0; y < n; ++y)
    {
        for (int x = 0; x < n; ++x)
        {
            augmented[y][x] = matrix[y][x]; // Copy original matrix
        }
        augmented[y][n + y] = 1.0; // Identity matrix on the right
    }

    // Perform row operations to transform [A | I] -> [I | A^-1]
    for (int i = 0; i < n; ++i)
    {
        // Find the pivot row
        float pivot = augmented[i][i];
        if (pivot == 0.0)
        {
            Serial.println("Error gaussJordanInverse: Singular matrix (non-invertible).");
            return Matrix(n, n); // Return original matrix as a fallback
        }

        // Normalize pivot row
        for (int j = 0; j < 2 * n; ++j)
        {
            augmented[i][j] /= pivot;
        }

        // Eliminate all other entries in column
        for (int k = 0; k < n; ++k)
        {
            if (k != i)
            {
                float factor = augmented[k][i];
                for (int j = 0; j < 2 * n; ++j)
                {
                    augmented[k][j] -= factor * augmented[i][j];
                }
            }
        }
    }

    // Extract the inverse matrix from the augmented matrix
    Matrix inverse(n, n);
    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; ++j)
        {
            inverse[i][j] = augmented[i][n + j]; // Copy the right half
        }
    }

    return inverse;
}

/**
 * @brief Compute the QR decomposition of the matrix
 *
 * @return std::pair<Matrix, Matrix> The Q and R matrices
 */
std::pair<Matrix, Matrix> Matrix::qrDecomposition() const
{
    if (rows() == 0 || cols() == 0)
    {
        Serial.println("Error qrDecomposition: Matrix is empty.");
        return {Matrix(rows(), cols()), Matrix(cols(), cols())};
    }

    // Initialize correct sizes
    Matrix Q(rows(), cols());
    Matrix R(cols(), cols());

    // Perform Gram-Schmidt
    for (int j = 0; j < cols(); ++j)
    {

        // Step 1: Extract j-th column of A
        Matrix v = getColumn(j);

        // Step 2: Orthogonalization
        for (int k = 0; k < j; ++k)
        {
            float dot = 0;
            for (int i = 0; i < rows(); ++i)
            {
                dot += Q[i][k] * matrix[i][j];
            }
            R[k][j] = dot;
            for (int i = 0; i < rows(); ++i)
            {
                v[i][0] -= dot * Q[i][k];
            }
        }

        // Step 3: Normalize
        float norm = 0;
        for (int i = 0; i < rows(); ++i)
        {
            norm += v[i][0] * v[i][0];
        }
        norm = sqrt(norm);

        if (norm == 0)
        {
            Serial.println("Error qrDecomposition: Zero norm encountered during QR decomposition.");
            return {Matrix(rows(), cols()), Matrix(cols(), cols())};
        }

        R[j][j] = norm;
        for (int i = 0; i < rows(); ++i)
        {
            Q[i][j] = v[i][0] / norm;
        }
    }
    return {Q, R};
}

/**
 * @brief Compute the inverse of the matrix using QR decomposition
 *
 * @return Matrix The inverse of the matrix
 */
Matrix Matrix::inverseQR() const
{
    int n = rows();

    // Check if the matrix is square
    if (n != cols())
    {
        Serial.println("Error inverseQR: Matrix must be square to compute inverse.");
        return Matrix(rows(), cols());
    }

    for (int i = 0; i < n; ++i)
    {
        if (matrix[i][i] == 0.0)
        {
            Serial.println("Error inverseQR: Singular matrix detected during Gauss-Jordan elimination.");
            return Matrix(n, n);
        }
    }

    // Perform QR decomposition
    std::pair<Matrix, Matrix> qrResult = qrDecomposition();
    Matrix Q = qrResult.first;
    Matrix R = qrResult.second;

    // Initialize inverse matrix
    Matrix inverse(n, n);

    // Solve R * X = Q^T * I (column-wise)
    Matrix Qt = Q.transpose();
    for (int j = 0; j < n; ++j)
    {
        Matrix e(n, 1);
        e[j][0] = 1.0; // Column of identity matrix

        Matrix b = Qt * e; // Compute Q^T * e_j

        // Back-substitution to solve R * x = b
        for (int i = n - 1; i >= 0; --i)
        {
            if (R.matrix[i][i] == 0.0)
            {
                Serial.println("Error inverseQR: Singular matrix in upper triangular solve.");
                return Matrix(n, n);
            }

            float sum = 0.0;
            for (int k = i + 1; k < n; ++k)
            {
                sum += R.matrix[i][k] * inverse[k][j];
            }
            inverse[i][j] = (b[i][0] - sum) / R.matrix[i][i];
        }
    }

    return inverse;
}

/**
 * @brief Compute the eigenvalues and eigenvectors of a symmetric matrix using the Jacobi method.
 *
 * @param maxIterations The maximum number of iterations.
 * @param tolerance The tolerance for convergence.
 *
 * @return A pair containing:
 * - First: A column vector (Nx1 matrix) of eigenvalues.
 * - Second: An NxN matrix whose columns are the corresponding eigenvectors.
 */
std::pair<Matrix, Matrix> Matrix::eigenJacobi(int maxIterations, float tolerance) const
{
    if (rows() != cols())
    {
        Serial.println("Error eigenJacobi: Matrix must be square to compute eigenvalues and eigenvectors.");
        return {Matrix(rows(), 1), Matrix(rows(), cols())};
    }

    int n = rows();
    Matrix A = *this; // Make a copy of the matrix to diagonalize.
    Matrix V(n, n);   // V will hold the eigenvectors.
    V.set_identity(1.0, n, 0, 0);

    for (int iter = 0; iter < maxIterations; ++iter)
    {
        // Find the largest off-diagonal element |A[p][q]|
        float maxVal = 0.0;
        int p = 0, q = 1; // Default initialization
        for (int i = 0; i < n; ++i)
        {
            for (int j = i + 1; j < n; ++j)
            {
                if (std::abs(A[i][j]) > maxVal)
                {
                    maxVal = std::abs(A[i][j]);
                    p = i;
                    q = j;
                }
            }
        }

        // Check for convergence
        if (maxVal < tolerance)
        {
            break;
        }

        // --- Cache the required values BEFORE modifying the matrix ---
        float app = A[p][p];
        float aqq = A[q][q];
        float apq = A[p][q];

        // Compute the Jacobi rotation parameters
        float theta = 0.5 * std::atan2(2.0 * apq, aqq - app);
        float c = std::cos(theta);
        float s = std::sin(theta);

        // --- Perform the rotation A' = J^T * A * J ---

        // CORRECTED: Update the diagonal elements with the correct formulas
        A[p][p] = c * c * app + s * s * aqq - 2.0 * c * s * apq;
        A[q][q] = s * s * app + c * c * aqq + 2.0 * c * s * apq;

        // Set the off-diagonal elements to zero
        A[p][q] = A[q][p] = 0.0;

        // Update the remaining elements in rows/columns p and q
        for (int k = 0; k < n; ++k)
        {
            if (k != p && k != q)
            {
                float akp = A[k][p];
                float akq = A[k][q];
                A[k][p] = A[p][k] = c * akp - s * akq;
                A[k][q] = A[q][k] = s * akp + c * akq;
            }
        }

        // --- Update the eigenvector matrix V' = V * J ---
        for (int k = 0; k < n; ++k)
        {
            float vkp = V[k][p];
            float vkq = V[k][q];
            V[k][p] = c * vkp - s * vkq;
            V[k][q] = s * vkp + c * vkq;
        }
    }

    // --- Sort the eigenvalues and eigenvectors based on the absolute value of the eigenvalues ---
    Matrix eigenvalues(n, 1); // Column vector for eigenvalues
    std::vector<std::pair<double, Matrix>> pairs;

    for (int i = 0; i < n; ++i)
    {
        double val = A[i][i];
        Matrix vec = V.getColumn(i);
        pairs.emplace_back(val, vec);
    }

    std::sort(pairs.begin(), pairs.end(),
              [](const std::pair<double, Matrix> &a, const std::pair<double, Matrix> &b)
              {
                  return std::abs(a.first) > std::abs(b.first);
              });

    for (int i = 0; i < n; ++i)
    {
        eigenvalues[i][0] = pairs[i].first;
        for (int j = 0; j < n; ++j)
        {
            V[j][i] = pairs[i].second[j][0];
        }
    }

    return {eigenvalues, V};
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
 * @brief Compute the covariance matrix of a given matrix A.
 *
 * @param A Input matrix (m x n)
 * @return Matrix Covariance matrix (n x n)
 */
Matrix covariance(const Matrix &A)
{
    int m = A.rows();
    int n = A.cols();

    // Compute the mean of each column
    Matrix mean(1, n);
    for (int j = 0; j < n; ++j)
    {
        float sum = 0;
        for (int i = 0; i < m; ++i)
        {
            sum += A[i][j];
        }
        mean[0][j] = sum / m;
    }

    // Compute the covariance matrix
    Matrix cov(n, n);
    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; ++j)
        {
            float sum = 0;
            for (int k = 0; k < m; ++k)
            {
                sum += (A[k][i] - mean[0][i]) * (A[k][j] - mean[0][j]);
            }
            cov[i][j] = sum / m;
        }
    }

    return cov;
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