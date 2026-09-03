#pragma once

template <typename ItemType> class Slot
{
public:
    Slot() = default;

    void setItem(ItemType item);
    ItemType& getItem();
    const ItemType& getItem() const;
    void incrementGeneration();
    unsigned int getGeneration();
    unsigned int getGeneration() const;
    void setGeneration(unsigned int generation);

private:
    ItemType item;
    unsigned int generation = 0;
};
