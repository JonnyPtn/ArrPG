#include "PlayerController.hpp"
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
#include <xygine/Scene.hpp>
#include "Messages.hpp"
#include "WorldController.hpp"
#include <xygine/util/Random.hpp>

PlayerController::PlayerController(xy::MessageBus& mb) :
	xy::Component(mb, this)
{
}

void PlayerController::onStart(xy::Entity& entity)
{
    //get the position in the world
    
    //check if we're on land or sea and load appropriate component


    //hull
    auto hull = xy::Component::create < xy::SfDrawableComponent<sf::RectangleShape>>(getMessageBus());
    hull->getDrawable().setFillColor({ 244,164,96 }); //brown ship
    hull->getDrawable().setSize({ 40.f, 100.f }); //of reasonable size
    auto& tex = m_textures.get("Ship.png");
    hull->getDrawable().setTexture(&tex);
    hull->getDrawable().setTextureRect({ 0,0,int(tex.getSize().x),int(tex.getSize().y) });
    hull->getDrawable().setOrigin(20.f, 50.f);
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

    auto msg = getMessageBus().post<NewIslandData>(Messages::CreateIsland);
    msg->playerPosition = entity.getPosition();
    msg->seed = xy::Util::Random::value(0, std::numeric_limits<int>::max());

    xy::App::addUserWindow([&]()
    {
        if(ImGui::Button("Spawn Island"))
        {
            auto msg = getMessageBus().post<NewIslandData>(Messages::CreateIsland);
            msg->playerPosition = entity.getPosition();
            msg->seed = xy::Util::Random::value(0, std::numeric_limits<int>::max());
        }
    });
}

PlayerController::~PlayerController()
{
}

void PlayerController::entityUpdate(xy::Entity & entity, float dt)
{
    //get nearby islands
    auto pos = entity.getPosition();
    auto qtc = entity.getScene()->queryQuadTree({ pos.x - IslandDensity, pos.y - IslandDensity, IslandDensity * 2,IslandDensity * 2 });
    
    //find the nearest one
    float closestIsland(std::numeric_limits<float>::max());
    for (auto c : qtc)
    {
        auto e = c->getEntity();
        auto d = xy::Util::Vector::lengthSquared(pos - e->getPosition());
        if (d < closestIsland)
        {
            closestIsland = d;
            m_closestIsland = e;
        }
    }

    //if there isn't an island near enough, generate one
    if (closestIsland > std::pow(IslandDensity,2))
    {
       /* auto msg = getMessageBus().post<NewIslandData>(Messages::CreateIsland);
        msg->playerPosition = entity.getPosition();
        msg->seed = xy::Util::Random::value(0, std::numeric_limits<int>::max());*/
    }


    //kill lateral velocity
    auto currentRightNormal = m_body->getWorldVector({ 1,0 });
    auto lateralVelocity = xy::Util::Vector::dot(currentRightNormal, m_body->getLinearVelocity()) * currentRightNormal;
    auto impulse = m_body->getMass() * -lateralVelocity;
    m_body->applyLinearImpulse(impulse, m_body->getWorldCentre());


	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
	{
		m_body->applyForceToCentre(xy::Util::Vector::rotate({ 0,-100 }, entity.getRotation()));
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
