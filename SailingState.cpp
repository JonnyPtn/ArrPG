#include "SailingState.hpp"
#include "States.hpp"
#include <xygine/Entity.hpp>
#include <xygine/components/SfDrawableComponent.hpp>
#include <xygine/StateStack.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <xygine/App.hpp>
#include "ShipComponent.hpp"
#include "WindController.hpp"
#include <xygine/physics/RigidBody.hpp>
#include <xygine/physics/CollisionRectangleShape.hpp>
#include <xygine/physics/JointFriction.hpp>
#include <SFML/Window/Event.hpp>
#include <xygine/Resource.hpp>

SailingState::SailingState(xy::StateStack & stack, xy::State::Context& context)
	: xy::State(stack, context),
	m_messageBus(context.appInstance.getMessageBus()),
	m_scene(m_messageBus),
	m_physicsWorld(m_messageBus)
{
	//no gravity because top-down
	m_physicsWorld.setGravity({ 0.f,0.f });
	
	//boat
	auto boat = xy::Entity::create(m_messageBus);
	auto ship = xy::Component::create<ShipComponent>(m_messageBus,*boat);
	auto wind = xy::Component::create<WindController>(m_messageBus);
	boat->addComponent(ship);
	boat->addComponent(wind);
	m_boat = boat.get();
	m_scene.addEntity(boat,xy::Scene::Layer::FrontFront);

	//sea
	auto sea = xy::Entity::create(m_messageBus);
	auto seaComp = xy::Component::create < xy::SfDrawableComponent<sf::RectangleShape>>(m_messageBus);
	m_seaShape = &(seaComp->getDrawable());
	m_seaShape->setSize(context.defaultView.getSize());
	m_seaShape->setTextureRect({ 0,0,int(context.defaultView.getSize().x),int(context.defaultView.getSize().y) });
	m_seaShape->setTexture(&m_seaTexture.get("Sea.png"));
	m_seaShape->setOrigin(context.defaultView.getSize().x / 2, context.defaultView.getSize().y / 2);
	m_seaTexture.get("Sea.png").setRepeated(true);
	sea->addComponent(seaComp);
	m_scene.addEntity(sea, xy::Scene::Layer::BackRear);
}

SailingState::~SailingState()
{
}

bool SailingState::handleEvent(const sf::Event & evt)
{
	return false;
}

void SailingState::handleMessage(const xy::Message &message)
{
	m_scene.handleMessage(message);
}

bool SailingState::update(float dt)
{
	m_scene.update(dt);
	auto view = m_scene.getView();
	view.setCenter(m_boat->getWorldPosition());
	m_scene.setView(view);

	m_seaShape->setPosition(m_boat->getPosition());
	auto viewSize = getContext().defaultView.getSize();
	auto boatPos = m_boat->getPosition();
	sf::IntRect texRect( -viewSize.x / 2 + boatPos.x,-viewSize.y / 2 + boatPos.y,viewSize.x,viewSize.y );
	m_seaShape->setTextureRect(texRect);
	return false;
}

void SailingState::draw()
{
	auto& rt = getContext().renderWindow;
	rt.draw(m_scene);
	rt.draw(m_physicsWorld);
}

xy::StateID SailingState::stateID() const
{
	return States::Sailing;
}
