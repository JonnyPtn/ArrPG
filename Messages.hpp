#pragma once

#include <xygine/MessageBus.hpp>

enum Messages
{
    CREATE_ISLAND = xy::Message::Count,
    SEA_LEVEL_CHANGED,

    //player stuff
    BOAT_CHANGE
    
};