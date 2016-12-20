#pragma once
#include <xygine/State.hpp>
#include <xygine/StateStack.hpp>
#include <xygine/MessageBus.hpp>
#include <xygine/Scene.hpp>
#include <xygine/physics/World.hpp>
#include <xygine/Resource.hpp>

class SailingState : public xy::State
{
public:
	SailingState(xy::StateStack& stack , xy::State::Context& context);
	~SailingState();

	bool handleEvent(const sf::Event& evt) override;
	void handleMessage(const xy::Message&) override;
	bool update(float dt) override;
	void draw() override;
	xy::StateID stateID() const override;

private:
	xy::MessageBus&			m_messageBus;
	xy::Physics::World		m_physicsWorld;
	xy::Scene				m_scene;
	xy::Entity*				m_boat;
	sf::RectangleShape*		m_seaShape;
	xy::TextureResource		m_seaTexture;
};

