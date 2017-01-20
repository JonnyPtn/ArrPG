#pragma once
#include <xygine/components/Component.hpp>
#include <unordered_map>
#include <xygine/Resource.hpp>

#include "InventoryItem.hpp"
#include "Messages.hpp"

class InventoryComponent :public xy::Component
{
public:
    InventoryComponent(xy::MessageBus& mb, int maxSize);
    ~InventoryComponent();

    // Inherited via Component
    virtual xy::Component::Type type() const override;
    virtual void entityUpdate(xy::Entity &, float) override;
    virtual void onStart(xy::Entity&) override;

    const std::vector<std::unique_ptr<InventoryItem>>& getItems(); //get the items contained in this inventory

    template<class T>
    bool has(int count = 1) //does it have the specified number of this item
    {
        auto c = std::count_if(m_items.begin(), m_items.end(), [&](const std::unique_ptr<InventoryItem>& a)
        {
            return dynamic_cast<T*>(a.get()) != nullptr;
        });
        return c > 0;
    }


    void give(std::unique_ptr<InventoryItem>& item); //give this inventory an item

    std::unique_ptr<InventoryItem> take(const std::string& name); //take an item from this inventory

    int getMaxSize() const { return m_maxSize; }

private:
    std::vector<std::unique_ptr<InventoryItem>> m_items;
    int                                         m_maxSize;

};

