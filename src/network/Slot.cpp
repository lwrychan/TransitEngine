#include "Slot.hpp"
#include "network/AbstractNode.hpp"
#include "network/AbstractSegment.hpp"
#include "network/AbstractRoute.hpp"
#include "vehicle/Vehicle.hpp"

template<typename ItemType>
void Slot<ItemType>::setItem(ItemType item) {
    this->item = std::move(item);
}

template<typename ItemType>
ItemType& Slot<ItemType>::getItem() {
    return this->item;
}

template<typename ItemType>
void Slot<ItemType>::incrementGeneration() { this->generation++; }

template<typename ItemType>
unsigned int Slot<ItemType>::getGeneration() { return this->generation; }

template<typename ItemType>
void Slot<ItemType>::setGeneration(unsigned int generation) { this->generation = generation; }

template class Slot<network::AbstractNode>;
template class Slot<network::AbstractSegment>;
template class Slot<network::AbstractRoute>;
template class Slot<vehicle::Vehicle>;
