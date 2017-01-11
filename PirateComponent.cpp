#include "PirateComponent.hpp"

#include <xygine/components/AnimatedDrawable.hpp>
#include <xygine/physics/RigidBody.hpp>
#include <xygine/Entity.hpp>
#include <xygine/util/Vector.hpp>
#include <SFML/Window/Keyboard.hpp>
#include "Messages.hpp"

PirateComponent::PirateComponent(xy::MessageBus& mb, xy::Entity& entity, xy::Physics::RigidBody* body, xy::TextureResource& textures) :
    xy::Component(mb, this),
    m_textures(textures),
    m_body(body),
    m_active(false)
{
    //activate when landing
    xy::Component::MessageHandler handler;
    handler.id = Messages::ON_LAND_CHANGE;
    handler.action = [&](xy::Component* c, const xy::Message& msg)
    {
        if (m_active = msg.getData<bool>())
        {
            //just landed, kill momentum
            auto v = m_body->getLinearVelocity();
            auto impulse = v * m_body->getMass()*0.8f;
            m_body->applyLinearImpulse(-impulse, m_body->getWorldCentre());

            //force vertical
            m_body->setTransform(entity.getPosition(),0);
            m_body->setAngularVelocity(0.f);
        }
    };
    addMessageHandler(handler);
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
    if (m_active)
    {
        const float force(50.f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
        {
            m_body->applyForceToCentre({ 0,-force });
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        {
            m_body->applyForceToCentre({ -force, 0 });
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        {
            m_body->applyForceToCentre({ 0,force });
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        {
            m_body->applyForceToCentre({ force,0 });
        }
    }
}

void PirateComponent::onStart(xy::Entity & entity)
{
    //add the pirate
    auto pirate = xy::Component::create<xy::AnimatedDrawable>(getMessageBus());
    pirate->setTexture(m_textures.get("A_Pirate.png"));
    pirate->setFrameCount(6);
    pirate->setFrameRate(6);
    pirate->setFrameSize({ 32,32 });
    pirate->setLooped(true);
    pirate->play();
    pirate->setOrigin(16, 16);
    m_drawable = entity.addComponent(pirate);

    //add a physics object for him
   /* xy::Physics::CollisionRectangleShape pirateShape({ 32,32 });
    pirateShape.setDensity(1.0);
    pirateShape.setRestitution(1.0);
    m_colShape = m_body->addCollisionShape(pirateShape);*/

}

void PirateComponent::destroy()
{
    m_drawable->destroy();
   // m_colShape->destroy();
    Component::destroy();
}
