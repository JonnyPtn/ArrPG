#include "IslandComponent.hpp"

#include <xygine/Scene.hpp>
#include <xygine/components/SfDrawableComponent.hpp>
#include <xygine/physics/CollisionCircleShape.hpp>
#include <xygine/physics/RigidBody.hpp>
#include <SFML/Graphics/CircleShape.hpp>

IslandComponent::IslandComponent(xy::MessageBus& mb,xy::Physics::RigidBody* worldBody,sf::Vector2f position, float radius) :
	m_position(position),
	m_radius(radius),
	xy::Component(mb,this)
{
	m_drawable.setRadius(radius);
	m_drawable.setPosition(position);
	m_drawable.setFillColor(sf::Color::Green);

    xy::Physics::CollisionCircleShape islandShape(m_radius);
    islandShape.setRestitution(1.f);
    m_position += {m_radius, m_radius};
    islandShape.setPosition(m_position);
    islandShape.setDensity(1.f);
    worldBody->addCollisionShape(islandShape);
    
}

IslandComponent::~IslandComponent()
{
}

void IslandComponent::draw(sf::RenderTarget & target, sf::RenderStates states) const
{
    states.transform *= getTransform();
    target.draw(m_drawable,states);
}

sf::FloatRect IslandComponent::localBounds() const
{
	return m_drawable.getLocalBounds();
}

sf::FloatRect IslandComponent::globalBounds() const
{
	return m_drawable.getGlobalBounds();
}
