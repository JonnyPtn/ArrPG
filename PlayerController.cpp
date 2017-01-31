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

    //add the boat graphic
    auto boat = xy::Component::create<xy::SfDrawableComponent<sf::Sprite>>(getMessageBus());
    boat->getDrawable().setTexture(m_playerTextures.get("assets/Boat.png"));
    auto bounds = boat->globalBounds();
    boat->getDrawable().setOrigin(bounds.width / 2, bounds.height / 2);
    boat->getDrawable().scale(0.1f, 0.1f);

    //add the physics stuff
    auto body = xy::Component::create<xy::Physics::RigidBody>(getMessageBus(), xy::Physics::BodyType::Dynamic);
    body->setLinearDamping(1.f);
    body->setAngularDamping(1.f);
    xy::Physics::CollisionRectangleShape pirateShape({ boat->globalBounds().left,boat->globalBounds().top, boat->globalBounds().width,boat->globalBounds().height });
    pirateShape.setDensity(1.0);
    pirateShape.setRestitution(1.0);
    body->addCollisionShape(pirateShape);

    entity.addComponent(boat);
    entity.addComponent(body);

}

PlayerController::~PlayerController()
{
}

void PlayerController::entityUpdate(xy::Entity & entity, float dt)
{
    //get the physics body
    auto body = entity.getComponent<xy::Physics::RigidBody>();

    //kill lateral velocity
    auto currentRightNormal = body->getWorldVector({ 1,0 });
    auto lateralVelocity = xy::Util::Vector::dot(currentRightNormal, body->getLinearVelocity()) * currentRightNormal;
    auto impulse = body->getMass() * -lateralVelocity;
    body->applyLinearImpulse(impulse, body->getWorldCentre());

    //apply rudder force
    auto applyRudder = [&](bool turningLeft)
    {
        //max angle of the rudder in degrees
        const auto maxRudderAngle = 5.f;

        //get the velocity relative to the boat (i.e. the water velocity)
        auto forwardVector = body->getWorldVector({ 0,-1 });
        auto forwardVelocity = xy::Util::Vector::dot(forwardVector, body->getLinearVelocity()) * forwardVector;


        //rotate the velocity by the rudder amount
        auto v = xy::Util::Vector::rotate(forwardVelocity, turningLeft ? -maxRudderAngle : maxRudderAngle);

        //now get the change in momentum between the vectors
        auto dv = forwardVelocity - v;

        //and apply that impulse to the rudder position
        //which we'll assume to be the centre rear of the boat
        impulse = body->getMass()*dv;
        auto bounds = entity.globalBounds();
        auto impulsePos = body->getWorldCentre() + body->getWorldVector({ bounds.width / 2,bounds.height });
        body->applyForce(impulse, impulsePos);
    };

    //handle input
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
    {
        body->applyForceToCentre(xy::Util::Vector::rotate({ 0,-100 }, entity.getRotation()));
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {
        applyRudder(true);
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
        applyRudder(false);
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
