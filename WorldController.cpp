#include "WorldController.hpp"
#include "Messages.hpp"
#include <xygine/Entity.hpp>
#include <xygine/util/Random.hpp>
#include <xygine/util/Vector.hpp>
#include <xygine/Scene.hpp>
#include "IslandComponent.hpp"
#include <xygine/App.hpp>
#include <xygine/imgui/imgui.h>

WorldController::WorldController(xy::MessageBus& mb, int seed) :
    xy::Component(mb, this),
    m_seaLevel(0.5),
    m_worldSeed(seed)
{
    xy::Component::MessageHandler handler;
    handler.id = Messages::CREATE_ISLAND; 
    handler.action = [this](xy::Component* c, const xy::Message& msg)
    {
        auto& data = msg.getData<NewIslandData>();
        createIsland(data);
    };
    addMessageHandler(handler);

    //world debug settings
    xy::App::addUserWindow([this]()
    {
        if (ImGui::SliderFloat("Sea Level", &m_seaLevel, 0.f, 1.f))
        {
            auto msg = getMessageBus().post<float>(Messages::SEA_LEVEL_CHANGED);
            *msg = m_seaLevel;
        }
    });
}


WorldController::~WorldController()
{
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

void WorldController::createIsland(NewIslandData d)
{
    //set the island seed first
    xy::Util::Random::rndEngine.seed(d.seed);

    //find a suitable position for it
    auto closestIsland = std::numeric_limits<float>::max();
    auto pos = sf::Vector2f();

    //pick a random position within range of the player
    auto x = xy::Util::Random::value(d.playerPosition.x - IslandDensity, d.playerPosition.x + IslandDensity);
    auto y = xy::Util::Random::value(d.playerPosition.y - IslandDensity, d.playerPosition.y + IslandDensity);
    pos = { x,y };
    
    //create a new entity for the island
    auto island = xy::Entity::create(getMessageBus());
    island->setPosition(pos);
    auto islandController = xy::Component::create<IslandComponent>(getMessageBus(),xy::Util::Random::value(0,std::numeric_limits<int>::max()));
    island->addComponent(islandController);
    auto isleBounds = island->globalBounds();
    island->setOrigin(isleBounds.width / 2, isleBounds.height / 2);

    //check for collisions with any other islands
    bool done(true);
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
    auto bounds = island->globalBounds();
    m_islands.push_back(m_entity->getScene()->addEntity(island,xy::Scene::Layer::BackMiddle));
}
