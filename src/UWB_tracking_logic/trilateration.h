#ifndef TRILATERATION_H
#define TRILATERATION_H

#include <Arduino.h>
#include "matrix.h"
#include "leastSquare.h"
#include "KalmanFilter.h"

#define BUFFER_SIZE 10 // Number of stored data points

struct DataPoint
{
    float x, y, z;  // Coordinates of the anchor point
    float d;        // Distance to the target
};

/**
 * @brief Trilateration class.
 */
class trilateration
{
public:
    trilateration(int numOfDimensions = 3);
    void update(const DataPoint &point);
    Matrix getState() const;
    void printBuffer() const;

private:
    int numOfDimensions; // Number of dimensions (2D or 3D)
    int bufferIndex = 0; // Points to the next insertion position
    int count = 0;       // Number of data points in the buffer
    KalmanFilter kf;     // Kalman filter object
    DataPoint buffer[BUFFER_SIZE]; // Circular buffer for storing data points
};

#endif // TRILATERATION_H