#pragma once

#include <xygine/MessageBus.hpp>

enum Messages
{
    CREATE_ISLAND = xy::Message::Count,
    SEA_LEVEL_CHANGED,

    //player stuff
    ON_LAND_CHANGE
};

struct NewIslandData
{
    int seed;
    sf::Vector2f playerPosition;
};