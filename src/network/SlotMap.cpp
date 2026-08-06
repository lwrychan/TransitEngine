#include "SlotMap.hpp"
#include "Identifiers.hpp"
#include "network/Node.hpp"
#include "network/Segment.hpp"
#include "network/Route.hpp"
#include "vehicle/Vehicle.hpp"

template<typename ItemType, typename ItemIdType>
ItemIdType SlotMap<ItemType, ItemIdType>::add(ItemType item) {
    size_t newIndex;

    if (this->freeList.empty()) {
        this->slots.resize(this->slots.size() + 1);
        newIndex = this->slots.size() - 1;
    }
    else {
        newIndex = this->freeList.back();
        this->freeList.pop_back();
    }

    this->slots[newIndex].setItem(std::move(item));
    this->slots[newIndex].setGeneration(0);

    ItemIdType newId = ItemIdType{ static_cast<int>(newIndex), 0 };

    return newId;
}

template<typename ItemType, typename ItemIdType>
void SlotMap<ItemType, ItemIdType>::clear(size_t index) {
    if (index < 0 || index >= slots.size()) { return; }

    slots[index].incrementGeneration();
    freeList.push_back(index);
}

template<typename ItemType, typename ItemIdType>
ItemType& SlotMap<ItemType, ItemIdType>::get(ItemIdType itemId) {
    Slot<ItemType>& slot = this->slots[itemId.id];

    if (slot.getGeneration() == static_cast<unsigned int>(itemId.generation)) {
        return slot.getItem();
    }

    return slot.getItem();
}

template class SlotMap<network::Node, NodeId>;
template class SlotMap<network::Segment, SegmentId>;
template class SlotMap<network::Route, RouteId>;
template class SlotMap<vehicle::Vehicle, VehicleId>;
