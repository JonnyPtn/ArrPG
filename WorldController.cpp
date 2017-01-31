#include "WorldController.hpp"
#include "Messages.hpp"
#include <xygine/Entity.hpp>
#include <xygine/util/Random.hpp>
#include <xygine/util/Vector.hpp>
#include <xygine/Scene.hpp>
#include "IslandComponent.hpp"
#include <xygine/App.hpp>
#include <xygine/imgui/imgui.h>
#include <xygine/FileSystem.hpp>

//some constants for world behaviour, should probably be in settings or something.
//All values are in range 0-1

WorldController::WorldController(xy::MessageBus& mb) :
    xy::Component(mb, this),
    m_seaLevel(m_lowTide),
    m_lowTide(0.4f),
    m_highTide(0.7f),
    m_highAltitude(0.7f),
    m_dayLength(24 * 60), //24 IRL minutes == 1 in-game day
    m_worldTicks(0),
    m_currentTime(m_dayLength / 2), //It's hiiiigh nooon
    m_tidePhaseTime(m_dayLength / 2.1), //slightly more than 2 high tides a day, so it's a different time each day
    m_tideIncoming(true),
    m_currentTideTime(0.f),
    m_saveFilePath()
{
    xy::Component::MessageHandler handler;
    handler.id = Messages::LOAD_WORLD;
    handler.action = [this](xy::Component* c, const xy::Message& msg)
    {
        auto& data = msg.getData<std::string>();
        load(data);
    };
    addMessageHandler(handler);

    handler.id = Messages::SAVE_WORLD;
    handler.action = [this](xy::Component* c, const xy::Message& msg)
    {
        auto& data = msg.getData<std::string>();
        m_saveFilePath = data;
        save();
    };
    addMessageHandler(handler);

    handler.id = Messages::CREATE_ISLAND; 
    handler.action = [this](xy::Component* c, const xy::Message& msg)
    {
        auto& data = msg.getData<std::pair<sf::Vector2f,int>>();
        createIsland(data.first,data.second);
    };
    addMessageHandler(handler);

    //world debug settings
    xy::App::addUserWindow([this]()
    {
        //show the current time of day
        ImGui::Text("Current Time: "); ImGui::SameLine();
        ImGui::Text((std::to_string(int(m_currentTime/(m_dayLength/24))) + ":").c_str()); ImGui::SameLine();
        ImGui::Text("%02d",int(m_currentTime/m_dayLength*(60*24))%60);

        //current sea level
        ImGui::Text("Current Sea Level (range 0-1): %f", m_seaLevel);
    });
}

WorldController::~WorldController()
{
}

void WorldController::entityUpdate(xy::Entity & entity, float dt)
{
    //deduct dt from time until tick
    m_currentTime += dt;

    //if it's a new day, tick and reset
    if (m_currentTime >= m_dayLength)
    {
        m_currentTime -= m_dayLength;
        m_worldTicks++;
    }


    //update the tide
    m_currentTideTime += dt;
    
    if (m_currentTideTime > m_tidePhaseTime)
    {
        m_currentTideTime -= m_tidePhaseTime;
        m_tideIncoming = !m_tideIncoming;
    }
    else
    {
        //update the sea level
        float startHeight = m_tideIncoming ? m_lowTide : m_highTide;
        float endHeight = m_tideIncoming ? m_highTide : m_lowTide;

        //interpolate for the current sea level
        m_seaLevel = startHeight + ((endHeight - startHeight)*m_currentTideTime / m_tidePhaseTime);
        auto msg = getMessageBus().post<float>(Messages::SEA_LEVEL_CHANGED);
        *msg = m_seaLevel;
    }
}

void WorldController::onStart(xy::Entity & ent)
{
    m_entity = &ent;
}

bool WorldController::isLand(const sf::Vector2f position)
{
    //first find if it's on an island or not
    for (auto island : m_islands)
    {
        if (island->globalBounds().contains(position))
        {
            //it's on this island, check the island
            auto component = island->getComponent<IslandComponent>();
            return component->isLand(position);
        }
    }
    return false;
}

float WorldController::getWorldTimeOfDay()
{
    return m_currentTime / m_dayLength;
}

void WorldController::createIsland(sf::Vector2f position, int seed)
{    
    //create a new entity for the island
    auto island = xy::Entity::create(getMessageBus());
    island->setPosition(position);
    auto islandController = xy::Component::create<IslandComponent>(getMessageBus(),seed,m_lowTide,m_highTide,m_highAltitude,m_seaLevel);
    island->addComponent(islandController);
    auto isleBounds = island->globalBounds();
    island->setOrigin(isleBounds.width / 2, isleBounds.height / 2);

    //check for collisions with any other islands
   /* bool done(true);
    do
    {
        done = true;
        for (auto i: m_islands)
        {
            auto igb = i->globalBounds();
            isleBounds = island->globalBounds();
            if (igb.intersects(isleBounds))
            {
                auto x = xy::Util::Random::value(d.playerPosition.x - IslandDensity, d.playerPosition.x + IslandDensity);
                auto y = xy::Util::Random::value(d.playerPosition.y - IslandDensity, d.playerPosition.y + IslandDensity);
                pos = { x,y };
                island->setPosition(pos);
                done = false;
                break;
            }
        }
    } while (!done);
    auto bounds = island->globalBounds();*/
    m_islands.push_back(m_entity->getScene()->addEntity(island,xy::Scene::Layer::BackMiddle));
}

void WorldController::save()
{
    //make sure saves directory exists
    if(!xy::FileSystem::directoryExists("saves"))
        xy::FileSystem::createDirectory("saves");

    std::ofstream file("saves/" + m_saveFilePath, std::ios::app);
    file.write(reinterpret_cast<char*>(&m_worldSeed),sizeof(m_worldSeed));

    for (auto i : m_islands)
    {
        //just save the position and the seed, the rest is procedural
        auto isleComp = i->getComponent<IslandComponent>();

        auto seed = isleComp->getSeed();
        file.write(reinterpret_cast<char*>(&seed), sizeof(seed));

        auto pos = i->getPosition();
        file.write(reinterpret_cast<char*>(&pos.x), sizeof(pos.x));
        file.write(reinterpret_cast<char*>(&pos.y), sizeof(pos.y));
    }
}

void WorldController::load(const std::string& saveFilePath)
{
    m_saveFilePath = saveFilePath;
    std::ifstream saveFile("saves/" + m_saveFilePath, std::ios::binary);

    saveFile.read(reinterpret_cast<char*>(&m_worldSeed), sizeof(m_worldSeed));

  //  while (saveFile.good())
    {
        //must be some island data, spawn it
        int seed;
        saveFile.read(reinterpret_cast<char*>(&seed), sizeof(seed));

        sf::Vector2f pos;
        saveFile.read(reinterpret_cast<char*>(&pos.x), sizeof(pos.x));
        saveFile.read(reinterpret_cast<char*>(&pos.y), sizeof(pos.y));
        
        //have to post a message instead of calling the function directly otherwise it poos a brick
        auto msg = getMessageBus().post<std::pair<sf::Vector2f, int>>(Messages::CREATE_ISLAND);
        msg->first = pos;
        msg->second = seed;
    }
}

