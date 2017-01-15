#pragma once

#include <xygine/components/Component.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include "InventoryItem.hpp"
#include <xygine/Resource.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <xygine/Entity.hpp>

//perhaps BootyComponent?
class LootComponent : public xy::Component, public sf::Drawable
{
public:
    template<typename T>
    LootComponent(xy::MessageBus& mb,T item,xy::TextureResource& textures) :
        xy::Component(mb, this),
        m_item(std::make_unique<T>(item))
    {
        static_assert(std::is_base_of<InventoryItem, T>::value, "Must be derived from InventoryItem");
        sprite.setTexture(textures.get("assets/Inventory/" + item.m_name + ".png"));
    };
    ~LootComponent() {};

    // Inherited via Component
    virtual Type type() const override
    {
        return xy::Component::Type::Drawable;
    };
    void onStart(xy::Entity& ent) override
    {
        auto b = sprite.getLocalBounds();
        sprite.setOrigin(b.width / 2, b.height / 2);
        m_entity = &ent;
    }
    virtual void entityUpdate(xy::Entity &, float) override {};

    sf::FloatRect globalBounds() const override
    {
        return (sprite.getGlobalBounds());
    };

    // Inherited via Drawable
    virtual void draw(sf::RenderTarget & target, sf::RenderStates states) const override
    {
        target.draw(sprite, states);
    };

    //take the item from this loot
    std::unique_ptr<InventoryItem> take()
    {
        //destroy this entity and rturn the inventory item
        m_entity->destroy();
        return std::move(m_item);
    };
private:
    std::unique_ptr<InventoryItem>  m_item;
    sf::Sprite                      sprite;
    xy::Entity*                     m_entity;
};