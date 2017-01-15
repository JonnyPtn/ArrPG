#include "InventoryUI.hpp"
#include <SFML/Graphics/RenderTarget.hpp>

const sf::Vector2f invGridCellSize(50, 50.f);

InventoryUI::InventoryUI(InventoryComponent& inventory) :
    xy::UI::Control(),
    m_inventory(inventory)
{
}


InventoryUI::~InventoryUI()
{
}

void InventoryUI::draw(sf::RenderTarget & target, sf::RenderStates states) const
{
    states.transform *= getTransform();
    for (auto& s : m_shapes)
    {
        target.draw(s, states);
    }
}

bool InventoryUI::selectable() const
{
    return false;
}

void InventoryUI::handleEvent(const sf::Event &, const sf::Vector2f &)
{
}

void InventoryUI::setAlignment(xy::UI::Alignment)
{
}

void InventoryUI::update()
{
    //clear the current display
    m_shapes.clear();
    //create a grid big enough for our inv size, as square as possible
    int size = std::ceil(std::sqrt(m_inventory.getMaxSize()));

    sf::Vector2i gridPos(0, 0);
    for (int i = 0; i < m_inventory.getMaxSize(); i++)
    {
        //add a background black rectangle for each one, white outline
        m_shapes.emplace_back();
        auto& s = m_shapes.back();
        s.setPosition(invGridCellSize.x*gridPos.x, invGridCellSize.y*gridPos.y);
        s.setOutlineThickness(-2);
        s.setSize(invGridCellSize);
        s.setFillColor(sf::Color::Transparent);
        s.setOutlineColor(sf::Color::Black);

        if (i < m_inventory.getItems().size())
        {
            //this slot is filled - get the texture
            auto& tex = m_textures.get("assets/inventory/" + m_inventory.getItems()[i]->m_name + ".png");
            s.setTexture(&tex);

            auto texSize = tex.getSize();
            s.setTextureRect({ 0, 0, int(texSize.x), int(texSize.y) });
            
            //make it visible
            s.setFillColor(sf::Color::White);

        }

        gridPos.x++;
        if (gridPos.x >= size)
        {
            gridPos.x = 0;
            gridPos.y++;
        }
    }
}
