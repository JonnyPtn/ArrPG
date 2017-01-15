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


    void give(std::unique_ptr<InventoryItem>& item) //give this inventory an item
    {
        //reject if we don't have enough room
        if (m_items.size() == m_maxSize)
            xy::Logger::log("Inventory not big enough for item", xy::Logger::Type::Error);
        else
        {
            //send off message
            auto msg = getMessageBus().post<std::string>(Messages::INVENTORY_CHANGE);
            *msg = item->m_name;

            m_items.emplace_back(std::move(item));
        }
    }

    template<class T>
    void take(int count = 1) //take an item from this inventory
    {
        while (count--)
        {
            m_items.erase(std::find_if(m_items.begin(), m_items.end(), [&](const std::unique_ptr<InventoryItem>& a)
            {
                return dynamic_cast<T*>(a.get()) != nullptr;
            }));
        }
    }

    int getMaxSize() const { return m_maxSize; }

private:
    std::vector<std::unique_ptr<InventoryItem>> m_items;
    int                                         m_maxSize;

};

