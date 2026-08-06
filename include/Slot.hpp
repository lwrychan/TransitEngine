#pragma once

template<typename ItemType>
class Slot {
public:
    Slot() = default;

    void setItem(ItemType item);
    ItemType& getItem();
    void incrementGeneration();
    unsigned int getGeneration();
    void setGeneration(unsigned int generation);
private:
    ItemType item;
    unsigned int generation = 0;
};
