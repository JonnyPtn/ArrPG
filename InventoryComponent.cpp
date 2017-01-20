#include "InventoryComponent.hpp"

#include <numeric>

InventoryComponent::InventoryComponent(xy::MessageBus& mb, int maxSize) :
    xy::Component(mb, this),
    m_maxSize(maxSize)
{
}


InventoryComponent::~InventoryComponent()
{
}

xy::Component::Type InventoryComponent::type() const
{
    return xy::Component::Type::Script;
}

void InventoryComponent::entityUpdate(xy::Entity &, float)
{
}

void InventoryComponent::onStart(xy::Entity &)
{
}

const std::vector<std::unique_ptr<InventoryItem>>& InventoryComponent::getItems()
{
    return m_items;
}

void InventoryComponent::give(std::unique_ptr<InventoryItem>& item)
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

std::unique_ptr<InventoryItem> InventoryComponent::take(const std::string & name)
{
    for (auto i = m_items.begin(); i!= m_items.end();)
    {
        if ((*i)->m_name == name)
        {
            auto toReturn = std::move(*i);
            i = m_items.erase(i);
            auto msg = getMessageBus().post<std::string>(Messages::INVENTORY_CHANGE);
            return std::move(toReturn);
        }
        else
            ++i;
    }
}
