#pragma once

#include <xygine/components/Component.hpp>
#include <xygine/Scene.hpp>
#include <xygine/Entity.hpp>
#include <SFML/Graphics/CircleShape.hpp>

class IslandComponent : public xy::Component, public sf::Drawable, public sf::Transformable
{
public:
	IslandComponent(xy::MessageBus& mb, xy::Physics::RigidBody* worldBody, sf::Vector2f position, float radius);
	~IslandComponent();

	Component::Type type() const {return Component::Type::Drawable;}
	void entityUpdate(xy::Entity& entity, float dt) {}

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	sf::FloatRect localBounds() const override;
	sf::FloatRect globalBounds() const override;

private:
    sf::CircleShape     m_drawable;
	sf::Vector2f	    m_position;
	float			    m_radius;
};

