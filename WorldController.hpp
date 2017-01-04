#pragma once

#include <xygine/components/Component.hpp>
#include <xygine/MessageBus.hpp>
#include "Messages.hpp"

constexpr float IslandDensity = 20000; //the minimum distance (ish) the player will ever be from an Island

class WorldController : public xy::Component
{
public:
    WorldController(xy::MessageBus& mb, int seed);
    ~WorldController();

    Component::Type type() const { return Component::Type::Script; }
    void entityUpdate(xy::Entity& entity, float dt) {}
    void onStart(xy::Entity& ent) override;

private:
    void createIsland(NewIslandData islandData);
    
    float m_seaLevel; //sea level range 0-1
    int m_worldSeed;
    std::vector<xy::Entity*> m_islands;
    xy::Entity* m_entity;
};

