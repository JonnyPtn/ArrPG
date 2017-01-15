#pragma once

#include <xygine/ui/Control.hpp>
#include "InventoryComponent.hpp"

class InventoryUI : public xy::UI::Control
{
public:
    InventoryUI(InventoryComponent& inventory);
    ~InventoryUI();

    // Inherited via Control
    virtual void draw(sf::RenderTarget & target, sf::RenderStates states) const override;
    virtual bool selectable() const override;
    virtual void handleEvent(const sf::Event &, const sf::Vector2f &) override;
    virtual void setAlignment(xy::UI::Alignment) override;

    void update();

private:
    InventoryComponent&                 m_inventory;
    xy::TextureResource                 m_textures;
    std::vector<sf::RectangleShape>    m_shapes;
};

