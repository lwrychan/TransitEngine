#include "Slot.hpp"
#include "network/Node.hpp"
#include "network/Segment.hpp"
#include "network/Route.hpp"
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

template class Slot<network::Node>;
template class Slot<network::Segment>;
template class Slot<network::Route>;
template class Slot<vehicle::Vehicle>;
