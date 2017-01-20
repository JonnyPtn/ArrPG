#include "PlayerController.hpp"
#include <xygine/Entity.hpp>
#include <xygine/components/SfDrawableComponent.hpp>
#include <xygine/physics/RigidBody.hpp>
#include <xygine/physics/CollisionEdgeShape.hpp>
#include <xygine/physics/CollisionRectangleShape.hpp>
#include <xygine/physics/CollisionCircleShape.hpp>
#include <xygine/physics/World.hpp>
#include <xygine/util/Vector.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <xygine/App.hpp>
#include <xygine/imgui/imgui.h>
#include <xygine/physics/JointHinge.hpp>
#include <xygine/physics/JointRope.hpp>
#include <xygine/Scene.hpp>
#include "Messages.hpp"
#include "WorldController.hpp"
#include <xygine/util/Random.hpp>
#include <xygine/components/AnimatedDrawable.hpp>
#include "BoatComponent.hpp"
#include "PirateComponent.hpp"
#include <xygine/imgui/imgui_sfml.h>
#include "LootComponent.hpp"
#include "InventoryComponent.hpp"

constexpr float viewDistance(15000.f);

PlayerController::PlayerController(xy::MessageBus& mb, WorldController& world) :
    xy::Component(mb, this),
    m_world(&world),
    m_closestIsland(nullptr)
{
}

void PlayerController::onStart(xy::Entity& entity)
{
    m_entity = &entity;
    //check if we're on land or sea and load appropriate component
   /* auto onLand = getMessageBus().post<bool>(Messages::BOAT_CHANGE);
    *onLand = m_world->isLand(entity.getWorldPosition());*/

    //add the pirate com
    auto pirate = xy::Component::create<PirateComponent>(getMessageBus(), entity, m_playerTextures);
    entity.addComponent(pirate);

    //DEBUG STUFF
    auto debugLine = xy::Component::create<xy::SfDrawableComponent<sf::VertexArray>>(getMessageBus());
    debugLine->getDrawable().setPrimitiveType(sf::Lines);

    m_debugLines = entity.addComponent(debugLine);

    //heightMap
    m_heightMapTexture.create(std::sqrt(m_nearestIslandHeightMap.size() / 4), std::sqrt(m_nearestIslandHeightMap.size() / 4));
    m_heightMapSprite.setTexture(m_heightMapTexture);

    //debug window
    xy::App::addUserWindow([&]()
    {
        if (m_closestIsland)
        {
            //get the island component
            auto island = m_closestIsland->getComponent<IslandComponent>();

            auto cc = island->getCellType(static_cast<sf::Vector2<double>>(entity.getPosition()));
            ImGui::Text("Current Biome: "); ImGui::SameLine();
            switch (cc)
            {
            case CellType::COAST:
                ImGui::Text("Coast");
                break;
            case CellType::HIGH_ALTITUDE:
                ImGui::Text("High Altitude");
                break;
            case CellType::LAND:
                ImGui::Text("Inland");
                break;
            case CellType::OCEAN:
                ImGui::Text("Ocean");
                kill();
                break;
            }

            //update the noise image
          /*  int iX = island->globalBounds().width;
            int iY = island->globalBounds().height;

            auto stride = std::sqrt(m_nearestIslandHeightMap.size()/4);
            for (int y = 0; y < stride;y++)
            {
                for (int x = 0; x < stride; x++)
                {
                    //get position relative to island
                    float relX = x / stride * iX;
                    float relY = y / stride * iY;

                    //get the height of the island at this place
                    auto h = island->getHeight({ relX, relY });

                    //update the pixel accordingly
                    m_nearestIslandHeightMap[(x + y*stride) * 4] = h * 255;
                    m_nearestIslandHeightMap[(x + y*stride) * 4+1] = h * 255;
                    m_nearestIslandHeightMap[(x + y*stride) * 4+2] = h * 255;
                    m_nearestIslandHeightMap[(x + y*stride) * 4+3] = 255; //just 100% alpha
                }
            }

            //update texture
            m_heightMapTexture.update(m_nearestIslandHeightMap.data());
            ImGui::Image(m_heightMapSprite);*/
        }
        else
        {
            kill();
            ImGui::Text("Current Biome: Ocean");
        }

        //quick debug button to spawn island on player
        if (ImGui::Button("Spawn Island"))
        {
            auto msg = getMessageBus().post<std::pair<sf::Vector2f,int>>(Messages::CREATE_ISLAND);
            msg->first = entity.getPosition();
            msg->second = xy::Util::Random::value(0, std::numeric_limits<int>::max());
        }
    });

}

PlayerController::~PlayerController()
{
}

void PlayerController::entityUpdate(xy::Entity & entity, float dt)
{
    //get nearest visible Island
    auto pos = entity.getPosition();
    auto qtc = entity.getScene()->queryQuadTree({ pos.x - viewDistance, pos.y - viewDistance, viewDistance * 2,viewDistance * 2 });

    //take out all the islands still in range
    for (auto i : qtc)
    {
        auto it = std::find(m_islandsInRange.begin(), m_islandsInRange.end(), i->getEntity());
        if (it != m_islandsInRange.end())
        {
            m_islandsInRange.erase(it);
        }
    }

    //we've removed all the ones still in range, any remaining have gone out of range, put them to sleep
    for (auto i : m_islandsInRange)
    {
        auto island = i->getComponent<IslandComponent>();
        island->setSleep(true);
    }

    //find the nearest one
    float closestIsland(std::numeric_limits<float>::max());
    for (auto c : qtc)
    {
        auto e = c->getEntity();

        //check if we're at least touching the entity
        //but only if we can reach it
        if (e->globalBounds().intersects(m_entity->globalBounds()))
        {
            //if it's an island
            auto i = e->getComponent<IslandComponent>();
            if (i)
            {
                auto e = c->getEntity();
                auto d = xy::Util::Vector::lengthSquared(pos - e->getPosition());
                if (d < closestIsland)
                {
                    closestIsland = d;
                    m_closestIsland = e;
                }
                //make sure this island is awake
                i->setSleep(false);
                m_islandsInRange.push_back(e);
            }

            //if it's loot, take it
            auto l = e->getComponent<LootComponent>();

            if (l)
            {
                if (l)
                {
                    auto inventory = m_entity->getComponent<InventoryComponent>();
                    inventory->give(l->take());
                }

            }
        }
    }

    //update the verts for the nearest island
   /* if (m_closestIsland)
        m_debugLines->getDrawable()[0].position = entity.getWorldTransform().getInverse().transformPoint(m_closestIsland->getPosition());
        */
    if (m_world)
    {
     /*   bool onLand = m_world->isLand(entity.getWorldPosition());
        if (onLand != m_onLand)
        {
            m_onLand = onLand;
        }*/
    }
}

void PlayerController::kill()
{
    //reset the position back to 0
    auto body = m_entity->getComponent<xy::Physics::RigidBody>();

    //dump inventory
    auto inv = m_entity->getComponent<InventoryComponent>();

    //gather item names
    std::vector<std::string> itemNames;
    for (auto& i : inv->getItems())
        itemNames.push_back(i->m_name);

    //gather items
    std::vector<std::unique_ptr<InventoryItem>> items;
    for (auto& i : itemNames)
        items.push_back(inv->take(i));

    //dump items as loot
    for (auto i = items.begin(); i!= items.end();i++)
    {
        //create the entity
        auto lootEntity = xy::Entity::create(getMessageBus());

        lootEntity->setPosition(m_entity->getPosition());

        //add the loot component, contains our loot (wood, would you beleive it!)
        auto lootComp = xy::Component::create<LootComponent>(getMessageBus(), std::move(*i), m_playerTextures);
        lootEntity->addComponent(lootComp);

        //add it to the quad tree too
        auto qtc = xy::Component::create<xy::QuadTreeComponent>(getMessageBus(), lootEntity->globalBounds());
        lootEntity->addComponent(qtc);

        //add to the scene
        m_entity->getScene()->addEntity(lootEntity,xy::Scene::Layer::BackMiddle);
    }

        
    body->setTransform({ 0,0 }, 0);
}
