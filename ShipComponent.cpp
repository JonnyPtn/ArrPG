#include "ShipComponent.hpp"
#include <xygine/Entity.hpp>
#include <xygine/components/SfDrawableComponent.hpp>
#include "WindController.hpp"
#include <xygine/physics/RigidBody.hpp>
#include <xygine/physics/CollisionEdgeShape.hpp>
#include <xygine/physics/CollisionRectangleShape.hpp>
#include <xygine/physics/World.hpp>
#include <xygine/util/Vector.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <xygine/App.hpp>
#include <xygine/imgui/imgui.h>
#include <xygine/physics/JointHinge.hpp>
#include <xygine/physics/JointRope.hpp>

ShipComponent::ShipComponent(xy::MessageBus& mb, xy::Entity& entity) :
	xy::Component(mb, this)
{
	
	//hull
	auto hull = xy::Component::create < xy::SfDrawableComponent<sf::RectangleShape>>(getMessageBus());
	hull->getDrawable().setFillColor({ 244,164,96 }); //brown ship
	hull->getDrawable().setSize({ 40.f, 100.f }); //of reasonable size
	auto& tex = m_textures.get("Ship.png");
	hull->getDrawable().setTexture(&tex);
	hull->getDrawable().setTextureRect({ 0,0,int(tex.getSize().x),int(tex.getSize().y) });
	hull->setOrigin(20.f, 50.f);
	entity.addComponent(hull);

	//sail
	auto sailComp = xy::Component::create < xy::SfDrawableComponent<sf::RectangleShape>>(getMessageBus());
	sailComp->getDrawable().setFillColor(sf::Color::White);
	sailComp->setPosition(-2.5, 0);
	sailComp->getDrawable().setSize({ 5.f, 50.f });

	//collision hull
	auto body = xy::Component::create<xy::Physics::RigidBody>(getMessageBus(), xy::Physics::BodyType::Dynamic);
	body->setLinearDamping(0.5f);
	body->setAngularDamping(0.5f);
	xy::Physics::CollisionRectangleShape hullShape({ -20,-50,40,100 });
	hullShape.setDensity(1.0);
	body->addCollisionShape(hullShape);
	m_body = entity.addComponent(body);

	//sail collision
	auto sail = xy::Entity::create(mb);
	auto sailBody = xy::Component::create<xy::Physics::RigidBody>(getMessageBus(), xy::Physics::BodyType::Dynamic);
	xy::Physics::CollisionRectangleShape sailShape({ { 0.05f,0.f, 0.1f,50.f } });
	sailShape.setDensity(50.f);
	sailShape.setRestitution(0.f);
	sailBody->addCollisionShape(sailShape);
	sail->addComponent(sailComp);
	m_sail = sail->addComponent(sailBody);
	m_sail->setAngularDamping(1000000000);
	entity.addChild(sail);
}

void ShipComponent::onStart(xy::Entity& entity)
{
	//hinge for sail
	xy::Physics::HingeJoint hinge(*m_body, { 0.f,0.f });
	auto pHinge = m_sail->addJoint(hinge);
	//pHinge->setLimits(-45.f, 45.f);
	//pHinge->limitEnabled(true);
}

ShipComponent::~ShipComponent()
{
}

void ShipComponent::entityUpdate(xy::Entity & entity, float dt)
{
	auto windVec = entity.getComponent<WindController>()->getWindVelocity();
//	m_body->applyForceToCentre(xy::Util::Vector::rotate({ 0,-xy::Util::Vector::length(windVec) }, entity.getRotation()));
	m_sail->applyForceToCentre(windVec);

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
	{
		m_body->applyForceToCentre(xy::Util::Vector::rotate({ 0,-xy::Util::Vector::length(windVec) }, entity.getRotation()));
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
	{
		m_body->applyTorque(20);
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
	{
		m_body->applyTorque(-20);
	}
}
