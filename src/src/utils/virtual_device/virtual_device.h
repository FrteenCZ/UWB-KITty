#ifndef VIRTUAL_DEVICE_H
#define VIRTUAL_DEVICE_H

#include <unordered_map>
#include <string>
#include <vector>
#include "../../UWB_tracking_logic/trilateration.h"
#include "../../UWB_tracking_logic/matrix.h"

std::unordered_map<std::string, class VirtualDevice> virtualDevices;

class VirtualDevice
{
public:
    trilateration trilat;

    VirtualDevice(int dimensions) : numOfDimensions(dimensions) {}

    void setAnchor(const std::string &selfId,
                   const std::vector<std::pair<std::string, float>> &distances);

    void update(const std::string &selfId,
                const std::vector<std::pair<std::string, float>> &distances);

private:
    int numOfDimensions;

    Matrix resolveNullspaceDirection(const Matrix &solution,
                                     const Matrix &null_space,
                                     float alpha,
                                     const Matrix &referenceDir);
};

#endif // VIRTUAL_DEVICE_H