#include "PirateComponent.hpp"

#include <xygine/components/AnimatedDrawable.hpp>
#include <xygine/physics/RigidBody.hpp>
#include <xygine/Entity.hpp>
#include <xygine/util/Vector.hpp>
#include <SFML/Window/Keyboard.hpp>
#include "Messages.hpp"

PirateComponent::PirateComponent(xy::MessageBus& mb, xy::Entity& entity, xy::TextureResource& textures) :
    xy::Component(mb, this),
    m_textures(textures),
    m_onBoat(false)
{
}


PirateComponent::~PirateComponent()
{
}

xy::Component::Type PirateComponent::type() const
{
    return xy::Component::Type::Script;
}

void PirateComponent::entityUpdate(xy::Entity & ent, float)
{
    if (m_onBoat)
        return;
    else
    {
        auto body = ent.getComponent<xy::Physics::RigidBody>();
        auto force = 200.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
        {
            body->applyForceToCentre({ 0,-force });
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        {
            body->applyForceToCentre({ -force, 0 });
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        {
            body->applyForceToCentre({ 0,force });
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        {
            body->applyForceToCentre({ force,0 });
        }
    }
}

void PirateComponent::onStart(xy::Entity & entity)
{
    //activate when landing
    xy::Component::MessageHandler handler;
    handler.id = Messages::BOAT_CHANGE;
    handler.action = [&](xy::Component* c, const xy::Message& msg)
    {
        if (m_onBoat = msg.getData<bool>())
        {
        }
    };
    addMessageHandler(handler);

    //add the pirate drawable
    auto pirate = xy::Component::create<xy::AnimatedDrawable>(getMessageBus());
    pirate->setTexture(m_textures.get("A_Pirate.png"));
    pirate->setFrameCount(6);
    pirate->setFrameRate(6);
    pirate->setFrameSize({ 32,32 });
    pirate->setLooped(true);
    pirate->play();
    pirate->setOrigin(16, 16);
    m_drawable = entity.addComponent(pirate);

    //add a rigid body and shape for him
    auto body = xy::Component::create<xy::Physics::RigidBody>(getMessageBus(), xy::Physics::BodyType::Dynamic);
    body->setLinearDamping(10.f);
    xy::Physics::CollisionRectangleShape pirateShape({m_drawable->globalBounds().left,m_drawable->globalBounds().top, m_drawable->globalBounds().width,m_drawable->globalBounds().height });
    pirateShape.setDensity(1.0);
    pirateShape.setRestitution(1.0);
    body->addCollisionShape(pirateShape);
    entity.addComponent(body);

}

sf::FloatRect PirateComponent::globalBounds() const
{
    return m_drawable->globalBounds();
}

void PirateComponent::destroy()
{
    m_drawable->destroy();
   // m_colShape->destroy();
    Component::destroy();
}
