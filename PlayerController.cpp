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
    auto onLand = getMessageBus().post<bool>(Messages::ON_LAND_CHANGE);
    *onLand = m_world->isLand(entity.getWorldPosition());

    //add the body
    entity.addComponent(body);

    auto msg = getMessageBus().post<NewIslandData>(Messages::CREATE_ISLAND);
    msg->playerPosition = entity.getPosition();
    msg->seed = xy::Util::Random::value(0, std::numeric_limits<int>::max());

    xy::App::addUserWindow([&]()
    {
        //show the
    });

    //add the pirate and boat components - they'll sort their stuff out
    auto pirate = xy::Component::create<PirateComponent>(getMessageBus(), entity, m_body, m_playerTextures);
    entity.addComponent(pirate);
    auto boat = xy::Component::create<BoatComponent>(getMessageBus(), entity, m_body, m_playerTextures);
    entity.addComponent(boat);

}

PlayerController::~PlayerController()
{
}

void PlayerController::entityUpdate(xy::Entity & entity, float dt)
{
    //get nearest Island
    auto pos = entity.getPosition();
    auto qtc = entity.getScene()->queryQuadTree({ pos.x - IslandDensity, pos.y - IslandDensity, IslandDensity * 2,IslandDensity * 2 });

    //find the nearest one
    float closestIsland(std::numeric_limits<float>::max());
    for (auto c : qtc)
    {
        auto e = c->getEntity();
        auto d = xy::Util::Vector::lengthSquared(pos - e->getPosition());
        if (d < closestIsland)
        {
            closestIsland = d;
            m_closestIsland = e;
        }
    }

    if (m_world)
    {
        bool onLand = m_world->isLand(entity.getWorldPosition());
        if (onLand != m_onLand)
        {
            m_onLand = onLand;
            auto onLandMsg = getMessageBus().post<bool>(Messages::ON_LAND_CHANGE);
            *onLandMsg = m_onLand;
        }

        //get the current cell

    }
}
