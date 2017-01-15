#pragma once

#include <xygine/components/Component.hpp>
#include <xygine/MessageBus.hpp>
#include "Messages.hpp"

class WorldController : public xy::Component
{
public:
    WorldController(xy::MessageBus& mb);
    ~WorldController();

    Component::Type type() const { return Component::Type::Script; }
    void entityUpdate(xy::Entity& entity, float dt);
    void onStart(xy::Entity& ent) override;

    ///check if a world position is on land or not
    bool isLand(const sf::Vector2f position);

private:

    void createIsland(sf::Vector2f pos, int seed);
    
    int m_worldSeed;
    std::vector<xy::Entity*> m_islands; //Stores the island data for this world, along with their position
    xy::Entity* m_entity;

    std::string m_saveFilePath; //also kind of doubles as the world name

    void    save(); //save the world. To a hard drive, not like a superhero.
    void    load(); //load one of the aforementioned worlds you saved

    //world parameters
    //define how the world behaves
    float           m_dayLength;        //the IRL length of one in-game day in seconds
    unsigned int    m_worldTicks;       //keep track of the current world tick (day?) - for regen?
    float           m_currentTime;      //current time of day, in seconds
    const float     m_lowTide;          //everything below this is always ocean
    const float     m_highTide;         //everything below this but above lowtide is coast
    float           m_seaLevel;         //sea level range 0-1
    const float     m_highAltitude;     //everything above this is snow (remaining cells are land)
    float           m_tidePhaseTime;    //The time between high and low tide, IRL seconds
    float           m_currentTideTime;  //keeps track of the time of the current tide phase
    bool            m_tideIncoming;     //true if the tide is coming in, false otherwise
};

