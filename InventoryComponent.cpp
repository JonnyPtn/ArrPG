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