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

const std::vector<std::unique_ptr<InventoryItem>>& InventoryComponent::getItems()
{
    return m_items;
}

void InventoryComponent::give(std::unique_ptr<InventoryItem> item, int count)
{
    //reject if we don't have enough room
    if (m_items.size() + count > m_maxSize)
        xy::Logger::log("Inventory not big enough for item" + item->m_name, xy::Logger::Type::Error);
    else
    {
        m_items.emplace_back(std::move(item));
    }
}

bool InventoryComponent::has(std::unique_ptr<InventoryItem> item, int count)
{
    auto c = std::count_if(m_items.begin(), m_items.end(), [&](const std::unique_ptr<InventoryItem>& a)
    {
        return a->m_name == item->m_name;
    });
    return c > 0;
}

void InventoryComponent::take(std::unique_ptr<InventoryItem> item, int count)
{
    while (count--)
    {
        m_items.erase(std::find_if(m_items.begin(), m_items.end(), [&](const std::unique_ptr<InventoryItem>& a)
        {
            return a->m_name == item->m_name;
        }));
    }
}
