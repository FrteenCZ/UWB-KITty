#include "VirtualMachnies.h"

// --- VirtualMachine Implementation ---

VirtualMachine::VirtualMachine() : _id(0), _type(VM_OFF), _position({0,0,0}) {}

VirtualMachine::VirtualMachine(uint16_t id) : _id(id), _type(VM_OFF), _position({0,0,0})
{
}

void VirtualMachine::update(const std::vector<Measurement> &measurements, const std::map<uint16_t, VirtualMachine> &all_vms)
{
    // Let's prepare the Matrices
    int count = measurements.size();
    if (count == 0)
        return;

    Matrix cords(count, 3);
    Matrix dists(count, 1);

    int valid_points = 0;

    for (int i = 0; i < count; i++)
    {
        uint16_t anchor_id = measurements[i].anchor_id;

        // Lookup Anchor Position in the global list
        auto it = all_vms.find(anchor_id);
        if (it != all_vms.end())
        {
            const VirtualMachine &vm = it->second;

            // Only use if it is an Anchor
            if (vm.getType() == VM_ANCHOR) {
                VMPosition p = vm.getPosition();
                cords[valid_points][0] = p.x;
                cords[valid_points][1] = p.y;
                cords[valid_points][2] = p.z;

                dists[valid_points][0] = measurements[i].distance;

                valid_points++;
            }
        }
    }

    if (valid_points < 3) return;

    // Handle resizing if we skipped invalid anchors
    if (valid_points < count)
    {
        Matrix finalCords(valid_points, 3);
        Matrix finalDists(valid_points, 1);
        for(int k=0; k<valid_points; k++) {
             finalCords[k] = cords[k];
             finalDists[k] = dists[k];
        }
        _trilateration.update(finalCords, finalDists, millis());
    } 
    else 
    {
        _trilateration.update(cords, dists, millis());
    }
}

uint16_t VirtualMachine::getId() const
{
    return _id;
}

void VirtualMachine::setType(VMType type) {
    _type = type;
}

VMType VirtualMachine::getType() const
{
    return _type;
}

void VirtualMachine::setPosition(float x, float y, float z){
    _position = {x, y, z};
}

VMPosition VirtualMachine::getPosition() const{
    return _position;
}

// Get the current estimated state
trilateration VirtualMachine::getTrilateration() const{
    return _trilateration;
}

// --- VMManager Implementation ---

VMManager &VMManager::getInstance()
{
    static VMManager instance;
    return instance;
}

VirtualMachine* VMManager::addVM(uint16_t id, VMType type)
{
    _vms[id] = VirtualMachine(id);
    _vms[id].setType(type);
    return &_vms[id];
}

bool VMManager::removeVM(uint16_t id)
{
    return _vms.erase(id) > 0;
}

VirtualMachine *VMManager::processMeasurements(uint16_t tag_id, const std::vector<Measurement> &measurements)
{
    // Find or Create Tag
    if (_vms.find(tag_id) == _vms.end())
    {
        addVM(tag_id, VM_TAG);
    }

    VirtualMachine &tag = _vms.at(tag_id);

    // Update the tag with new data
    tag.update(measurements, _vms);

    return &tag;
}

VirtualMachine *VMManager::getVM(uint16_t id)
{
    if (_vms.find(id) != _vms.end())
    {
        return &_vms.at(id);
    }
    return nullptr;
}