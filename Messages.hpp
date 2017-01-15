#pragma once

#include <xygine/MessageBus.hpp>

enum Messages
{
    CREATE_ISLAND = xy::Message::Count,
    SEA_LEVEL_CHANGED,
    LOAD_WORLD,
    SAVE_WORLD,

    //player stuff
    BOAT_CHANGE,
    INVENTORY_CHANGE,
    
};