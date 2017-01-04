#pragma once

#include <xygine/components/Component.hpp>
#include <xygine/Resource.hpp>
#include <xygine/Entity.hpp>

class PlayerController : public xy::Component
{
public:
	PlayerController(xy::MessageBus& mb);
	~PlayerController();
	void onStart(xy::Entity& entity) override;
	void entityUpdate(xy::Entity& entity, float dt) override;

	xy::Component::Type type() const { return xy::Component::Type::Script; }

private:
	xy::Physics::RigidBody*	m_body;
	xy::Physics::RigidBody* m_sail;
	xy::Entity*	m_sailE;
	xy::Entity*	m_hullE;
	xy::TextureResource		m_textures;
    xy::Entity* m_closestIsland;
};

