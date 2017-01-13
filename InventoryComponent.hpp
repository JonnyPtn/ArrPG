#pragma once
#include <xygine/components/Component.hpp>
#include <unordered_map>

#include "InventoryItem.hpp"

class InventoryComponent :public xy::Component
{
public:
    InventoryComponent(xy::MessageBus& mb, int maxSize);
    ~InventoryComponent();

    // Inherited via Component
    virtual Type type() const override;
    virtual void entityUpdate(xy::Entity &, float) override;

    const std::vector<std::unique_ptr<InventoryItem>>& getItems(); //get the items contained in this inventory

    bool has(std::unique_ptr<InventoryItem> item, int count = 1); //does it have the specified number of this item
    void give(std::unique_ptr<InventoryItem> item, int count = 1); //give this inventory an item
    void take(std::unique_ptr<InventoryItem> item, int count = 1);//take an item from this inventory

private:
    std::vector<std::unique_ptr<InventoryItem>> m_items;
    int                                         m_maxSize;
};

