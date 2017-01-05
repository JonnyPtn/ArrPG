#include "BoatComponent.hpp"

#include <xygine/components/SfDrawableComponent.hpp>
#include <xygine/physics/RigidBody.hpp>
#include <xygine/util/Vector.hpp>
#include <SFML/Window/Keyboard.hpp>

BoatComponent::BoatComponent(xy::MessageBus& mb, xy::Entity& entity, xy::Physics::RigidBody* body, xy::TextureResource& textures) :
    xy::Component(mb, this),
    m_textures(textures),
    m_body(body)
{
    m_drawable.setFillColor({ 244,164,96 }); //brown ship
    m_drawable.setSize({ 40.f, 100.f }); //of reasonable size
    auto& tex = m_textures.get("Ship.png");
    m_drawable.setTexture(&tex);
    m_drawable.setTextureRect({ 0,0,int(tex.getSize().x),int(tex.getSize().y) });
    m_drawable.setOrigin(20.f, 50.f);
}

void BoatComponent::onDelayedStart(xy::Entity& entity)
{

    //collision hull
    /*xy::Physics::CollisionRectangleShape hullShape({ -20,-50, 40,100 });
    hullShape.setDensity(1.0);
    hullShape.setRestitution(1.0);
    m_colShape = m_body->addCollisionShape(hullShape);*/

    auto v = m_body->getLinearVelocity();

    //kill any momentum
    auto impulse = v * m_body->getMass();
    m_body->applyLinearImpulse(-impulse, m_body->getWorldCentre());

    //rotate to match velocity direction
    auto angle = xy::Util::Vector::rotation(v) + 90;
    m_body->setRotation(angle);
}

void BoatComponent::draw(sf::RenderTarget & target, sf::RenderStates states) const
{
    states.transform *= getTransform();
    target.draw(m_drawable, states);
}

BoatComponent::~BoatComponent()
{
}

xy::Component::Type BoatComponent::type() const
{
    return xy::Component::Type::Drawable;
}

void BoatComponent::entityUpdate(xy::Entity & entity, float)
{
    //get nearby islands
    /*auto pos = entity.getPosition();
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
    if (closestIsland > std::pow(IslandDensity, 2))
    {
         auto msg = getMessageBus().post<NewIslandData>(Messages::CreateIsland);
        msg->playerPosition = entity.getPosition();
        msg->seed = xy::Util::Random::value(0, std::numeric_limits<int>::max());
    }*/

    auto body = entity.getComponent<xy::Physics::RigidBody>();

    //kill lateral velocity
    auto currentRightNormal = body->getWorldVector({ 1,0 });
    auto lateralVelocity = xy::Util::Vector::dot(currentRightNormal, body->getLinearVelocity()) * currentRightNormal;
    auto impulse = body->getMass() * -lateralVelocity;
    body->applyLinearImpulse(impulse, body->getWorldCentre());


    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
    {
        body->applyForceToCentre(xy::Util::Vector::rotate({ 0,-100 }, entity.getRotation()));
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {
        body->applyTorque(1);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
        body->applyTorque(-1);
    }
}
