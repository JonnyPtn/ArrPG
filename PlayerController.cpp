#include "PlayerController.hpp"
#include <xygine/Entity.hpp>
#include <xygine/components/SfDrawableComponent.hpp>
#include "WindController.hpp"
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

PlayerController::PlayerController(xy::MessageBus& mb, WorldController& world) :
    xy::Component(mb, this),
    m_world(&world)
{
}

void PlayerController::onStart(xy::Entity& entity)
{    
    //we need a rigid body
    auto body = xy::Component::create<xy::Physics::RigidBody>(getMessageBus(), xy::Physics::BodyType::Dynamic);
    m_body = body.get();
    m_body->setLinearDamping(0.5f);
    m_body->setAngularDamping(10.f);

    //and a fixture for it because of some bullcrap with dynamic fixture management
    auto fixture = xy::Physics::CollisionCircleShape(20);
    fixture.setDensity(1.0);
    m_body->addCollisionShape(fixture);

    //check if we're on land or sea and load appropriate component
    if (onLand = m_world->isLand(entity.getPosition()))
    {
        auto pirate = xy::Component::create<PirateComponent>(getMessageBus(), entity, m_body, m_playerTextures);
        entity.addComponent(pirate);
    }
    else
    {
        auto boat = xy::Component::create<BoatComponent>(getMessageBus(), entity, m_body,m_playerTextures);
        entity.addComponent(boat);
    }

    //recenter the bounds
    //auto bounds = entity.globalBounds();
    //entity.setOrigin(bounds.width / 2, bounds.height / 2);

    //add the body
    entity.addComponent(body);

    auto msg = getMessageBus().post<NewIslandData>(Messages::CreateIsland);
    msg->playerPosition = entity.getPosition();
    msg->seed = xy::Util::Random::value(0, std::numeric_limits<int>::max());

    xy::App::addUserWindow([&]()
    {
        if(ImGui::Button("Spawn Island"))
        {
            auto msg = getMessageBus().post<NewIslandData>(Messages::CreateIsland);
            msg->playerPosition = entity.getPosition();
            msg->seed = xy::Util::Random::value(0, std::numeric_limits<int>::max());
        }
    });
}

PlayerController::~PlayerController()
{
}

void PlayerController::entityUpdate(xy::Entity & entity, float dt)
{
    //if we're on land, go red
    if (m_world)
    {
        if (m_world->isLand(entity.getWorldPosition()))
        {
            //if we're a boat, we should be a pirate
            if (!onLand)
            {
                onLand = true;
                auto boat = entity.getComponent<BoatComponent>();
                boat->destroy();

                auto pirate = xy::Component::create<PirateComponent>(getMessageBus(),entity,m_body, m_playerTextures);
                entity.addComponent(pirate);
            }
        }
        else
        {
            //if we've just taken to sea, we're a boat
            if (onLand)
            {
                onLand = false;
                auto pirate = entity.getComponent<PirateComponent>();
                pirate->destroy();

                auto boat = xy::Component::create<BoatComponent>(getMessageBus(), entity,m_body, m_playerTextures);
                entity.addComponent(boat);
            }
        }
    }
}
