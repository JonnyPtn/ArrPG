#pragma once

#include <xygine/components/Component.hpp>
#include <xygine/MessageBus.hpp>

class WindController : public xy::Component
{
public:
	WindController(xy::MessageBus& mb);
	~WindController();

	xy::Component::Type type() const { return xy::Component::Type::Script; }

	void entityUpdate(xy::Entity& entity, float dt) override;

	sf::Vector2f getWindVelocity() const;

private:
	sf::Vector2f m_windVelocity;
};

