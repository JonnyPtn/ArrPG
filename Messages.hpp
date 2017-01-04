#pragma once

#include <xygine/MessageBus.hpp>

enum Messages
{
    CreateIsland = xy::Message::Count,
    SeaLevelChanged,
};

struct NewIslandData
{
    int seed;
    sf::Vector2f playerPosition;
};