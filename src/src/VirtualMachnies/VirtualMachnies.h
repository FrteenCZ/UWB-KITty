#ifndef VIRTUAL_MACHINES_H
#define VIRTUAL_MACHINES_H

#include <map>
#include <vector>
#include <Arduino.h>
#include "../UWB_tracking_logic/trilateration.h"

enum VMType
{
    VM_OFF,
    VM_ANCHOR,
    VM_TAG
};

struct VMPosition
{
    float x;
    float y;
    float z;
};

// Represents a measurement received from the external simulation
struct Measurement
{
    uint16_t anchor_id;
    float distance;
};

// Represents a virtual machine instance (either an Anchor or a Tag)
class VirtualMachine
{
public:
    VirtualMachine(); // Default constructor
    VirtualMachine(uint16_t id);
    
    // Process a set of measurements to update position
    // Requires access to all_vms to look up Anchor positions
    void update(const std::vector<Measurement> &measurements, const std::map<uint16_t, VirtualMachine> &all_vms);
    
    uint16_t getId() const;

    void setType(VMType type);
    VMType getType() const;

    void setPosition(float x, float y, float z);
    VMPosition getPosition() const;    

    // Get the current estimated state
    trilateration getTrilateration() const;

private:
    VMType _type;
    uint16_t _id;
    VMPosition _position;

    trilateration _trilateration;
};

// Manager class to hold the Constellation (Anchors) and Solvers (Tags)
class VMManager
{
public:
    static VMManager &getInstance();

    VirtualMachine* addVM(uint16_t id, VMType type = VM_ANCHOR);
    bool removeVM(uint16_t id);

    // Runtime: Handle incoming measurement data for a specific tag
    // Returns pointer to the updated Tag object (or creates one if new)
    VirtualMachine *processMeasurements(uint16_t tag_id, const std::vector<Measurement> &measurements);

    VirtualMachine *getVM(uint16_t id);

private:
    VMManager() {}

    std::map<uint16_t, VirtualMachine> _vms; // Database of active virtual machines
};

#endif