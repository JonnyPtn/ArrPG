#pragma once

#include <xygine/components/Component.hpp>
#include <xygine/Resource.hpp>
#include <xygine/Entity.hpp>
#include "WorldController.hpp"

class PlayerController : public xy::Component
{
public:
	PlayerController(xy::MessageBus& mb, WorldController&); //controller ref should be const but can't bc voronoi lib
	~PlayerController();
	void onStart(xy::Entity& entity) override;
	void entityUpdate(xy::Entity& entity, float dt) override;

	xy::Component::Type type() const { return xy::Component::Type::Script; }

private:
    xy::Entity*             m_closestIsland;
    WorldController*        m_world;
    xy::TextureResource     m_playerTextures;
    xy::Physics::RigidBody* m_body;
    bool onLand;
};

