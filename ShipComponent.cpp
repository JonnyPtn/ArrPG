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

	//collision hull
	auto body = xy::Component::create<xy::Physics::RigidBody>(getMessageBus(), xy::Physics::BodyType::Dynamic);
	body->setLinearDamping(0.5f);
	body->setAngularDamping(10.f);
	xy::Physics::CollisionRectangleShape hullShape({ -20,-50,40,100 });
	hullShape.setDensity(1.0);
    hullShape.setRestitution(1.0);
	body->addCollisionShape(hullShape);
	m_body = entity.addComponent(body);
}

void ShipComponent::onStart(xy::Entity& entity)
{
}

ShipComponent::~ShipComponent()
{
}

void ShipComponent::entityUpdate(xy::Entity & entity, float dt)
{
	auto windVec = entity.getComponent<WindController>()->getWindVelocity();
//	m_body->applyForceToCentre(xy::Util::Vector::rotate({ 0,-xy::Util::Vector::length(windVec) }, entity.getRotation()));
//	m_sail->applyForceToCentre(windVec);

    auto currentRightNormal = m_body->getWorldVector({ 1,0 });
    auto lateralVelocity = xy::Util::Vector::dot(currentRightNormal, m_body->getLinearVelocity()) * currentRightNormal;
    auto impulse = m_body->getMass() * -lateralVelocity;
    m_body->applyLinearImpulse(impulse, m_body->getWorldCentre());


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
