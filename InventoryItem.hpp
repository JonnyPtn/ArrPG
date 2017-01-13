#pragma once

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Drawable.hpp>

//virtual base class for Inventory Items.

class InventoryItem
{
public:
    const std::string&      m_name; //also used as icon asset name
};

//going to define the derived classes here for now bc lazy

class Wood : public InventoryItem
{
public:
    Wood() : InventoryItem({ "Wood" }) {};
};

class Rope : public InventoryItem
{
public:
    Rope() : InventoryItem({ "Rope" }) {};
};