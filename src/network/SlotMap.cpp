#include "SlotMap.hpp"
#include "Identifiers.hpp"
#include "network/AbstractNode.hpp"
#include "network/AbstractRoute.hpp"
#include "network/AbstractSegment.hpp"
#include "vehicle/Vehicle.hpp"

template <typename ItemType, typename ItemIdType>
ItemIdType SlotMap<ItemType, ItemIdType>::add(ItemType item)
{
    size_t newIndex;

    if (this->freeList.empty())
    {
        this->slots.resize(this->slots.size() + 1);
        newIndex = this->slots.size() - 1;
    }
    else
    {
        newIndex = this->freeList.back();
        this->freeList.pop_back();
    }

    this->slots[newIndex].setItem(std::move(item));
    this->slots[newIndex].setGeneration(0);

    ItemIdType newId = ItemIdType{static_cast<int>(newIndex), 0};

    return newId;
}

template <typename ItemType, typename ItemIdType>
size_t SlotMap<ItemType, ItemIdType>::getSlotCount()
{
    return this->slots.size();
}

template <typename ItemType, typename ItemIdType>
void SlotMap<ItemType, ItemIdType>::clear(size_t index)
{
    if (index < 0 || index >= slots.size())
    {
        return;
    }

    slots[index].incrementGeneration();
    freeList.push_back(index);
}

template <typename ItemType, typename ItemIdType>
ItemType& SlotMap<ItemType, ItemIdType>::get(ItemIdType itemId)
{
    Slot<ItemType>& slot = this->slots[itemId.id];

    if (slot.getGeneration() == static_cast<unsigned int>(itemId.generation))
    {
        return slot.getItem();
    }

    return slot.getItem();
}

template <typename ItemType, typename ItemIdType>
const ItemType& SlotMap<ItemType, ItemIdType>::get(ItemIdType itemId) const
{
    const Slot<ItemType>& slot = this->slots[itemId.id];
    return slot.getItem();
}

template <typename ItemType, typename ItemIdType>
size_t SlotMap<ItemType, ItemIdType>::getActiveCount() const
{
    return this->slots.size() - this->freeList.size();
}

template <typename ItemType, typename ItemIdType>
void SlotMap<ItemType, ItemIdType>::forEachActive(
    const std::function<void(ItemIdType, ItemType&)>& callback)
{
    std::vector<bool> isFree(this->slots.size(), false);
    for (size_t index : this->freeList)
    {
        isFree[index] = true;
    }

    for (size_t index = 0; index < this->slots.size(); ++index)
    {
        if (isFree[index])
        {
            continue;
        }

        ItemIdType id{static_cast<int>(index),
                      static_cast<int>(this->slots[index].getGeneration())};
        callback(id, this->slots[index].getItem());
    }
}

template class SlotMap<network::AbstractNode, AbstractNodeId>;
template class SlotMap<network::AbstractSegment, AbstractSegmentId>;
template class SlotMap<network::AbstractRoute, AbstractRouteId>;
template class SlotMap<vehicle::Vehicle, VehicleId>;
