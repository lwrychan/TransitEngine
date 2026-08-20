#pragma once

#include <functional>
#include <vector>

#include "Slot.hpp"
#include "Identifiers.hpp"


template<typename ItemType, typename ItemIdType>
class SlotMap {
public:
    void clear(size_t index);

    ItemType& get(ItemIdType itemId);

    ItemIdType add(ItemType item);

    size_t getActiveCount() const;

    void forEachActive(const std::function<void(ItemIdType, ItemType&)>& callback);

private:
    std::vector<Slot<ItemType>> slots;
    std::vector<size_t> freeList;
};
