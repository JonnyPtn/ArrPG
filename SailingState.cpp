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
#include <xygine/util/Random.hpp>
#include "IslandComponent.hpp"

SailingState::SailingState(xy::StateStack & stack, xy::State::Context& context)
	: xy::State(stack, context),
	m_messageBus(context.appInstance.getMessageBus()),
	m_scene(m_messageBus),
	m_physicsWorld(m_messageBus),
    m_UIContainer(m_messageBus),
    m_snapToNorth(false)
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
	auto worldEnt = xy::Entity::create(m_messageBus);
	auto seaComp = xy::Component::create < xy::SfDrawableComponent<sf::RectangleShape>>(m_messageBus);
	m_seaShape = &(seaComp->getDrawable());
	m_seaShape->setSize(context.defaultView.getSize());
    m_seaShape->setScale({ 2.f,2.f });
	m_seaShape->setTextureRect({ 0,0,int(context.defaultView.getSize().x),int(context.defaultView.getSize().y) });
	m_seaShape->setTexture(&m_textures.get("Sea.png"));
	m_seaShape->setOrigin(context.defaultView.getSize().x / 2, context.defaultView.getSize().y / 2);
    m_textures.get("Sea.png").setRepeated(true);
	worldEnt->addComponent(seaComp);
  

    //world physics body
    auto worldBody = xy::Component::create<xy::Physics::RigidBody>(m_messageBus, xy::Physics::BodyType::Static);

	//generate islands in random positions
	auto fmax = 20000.f; //std::numeric_limits<float>::max();
	auto fmin = -20000.f; //std::numeric_limits<float>::min();

	//for now...
	auto maxRadius = 1000.f;
	auto minRadius = 100.f;
	auto islandCount = 500;
	while (islandCount--)
	{
		auto pos = sf::Vector2f(xy::Util::Random::value(fmin, fmax), xy::Util::Random::value(fmin, fmax));
		auto island = xy::Component::create<IslandComponent>(m_messageBus, worldBody.get(), pos, xy::Util::Random::value(minRadius, maxRadius));
		worldEnt->addComponent(island);
	}
    worldEnt->addComponent(worldBody);
	m_scene.addEntity(worldEnt, xy::Scene::Layer::BackRear);

    //UI

    //co-ordinates
    m_xPosDisplay = xy::UI::create<xy::UI::Label>(m_UIFonts.get("Westmeath.ttf"));
    m_yPosDisplay = xy::UI::create<xy::UI::Label>(m_UIFonts.get("Westmeath.ttf"));
    m_xPosDisplay->setAlignment(xy::UI::Alignment::TopLeft);
    m_yPosDisplay->setPosition({ 0,50 });
    m_UIContainer.addControl(m_xPosDisplay);
    m_UIContainer.addControl(m_yPosDisplay);

    //compass
    auto& compassTexture = m_textures.get("Compass.png");
    m_compass = xy::UI::create<xy::UI::Button>(m_UIFonts.get("Westmeath.ttf"),compassTexture);
    auto compassSize = compassTexture.getSize();
    m_compass->setPosition(sf::Vector2f( context.renderWindow.getSize().x - 100, 100 ));
    m_compass->setOrigin({ 32,96 });
    m_compass->addCallback([this]()
    {
        m_snapToNorth = !m_snapToNorth;
    });
    m_UIContainer.addControl(m_compass);
}

SailingState::~SailingState()
{
}

bool SailingState::handleEvent(const sf::Event & evt)
{
    switch (evt.type)
    {
    case sf::Event::MouseWheelScrolled:
    {
        auto view = m_scene.getView();
        view.zoom(1.f + evt.mouseWheelScroll.delta*0.1f);
        m_seaShape->scale({ 1.f + evt.mouseWheelScroll.delta*0.1f,1.f + evt.mouseWheelScroll.delta*0.1f });
        m_scene.setView(view);
        break;
    }
    }
    auto mousePos = sf::Mouse::getPosition(getContext().renderWindow);
    m_UIContainer.handleEvent(evt, sf::Vector2f(mousePos));
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
    auto boatPos = m_boat->getWorldPosition();
	view.setCenter(boatPos);
    if (m_snapToNorth)
    {
        m_compass->setRotation(-m_boat->getRotation());
        view.setRotation(m_boat->getRotation());
    }
    else
    {
        m_compass->setRotation(0);
        m_seaShape->setRotation(0);
        view.setRotation(0);
    }
	m_scene.setView(view);

    m_xPosDisplay->setString(std::to_string(boatPos.x/100.f));
    m_yPosDisplay->setString(std::to_string(boatPos.y/100.f));

	auto viewSize = m_scene.getView().getSize();
    m_seaShape->setPosition(m_scene.getView().getCenter());
    

    auto texRect = m_seaShape->getGlobalBounds();
    m_seaShape->setTextureRect(sf::IntRect(texRect));

    m_UIContainer.update(dt);
	return false;
}

void SailingState::draw()
{
    auto& rt = getContext().renderWindow;
    rt.draw(m_scene);
   // rt.draw(m_physicsWorld);

    rt.setView(rt.getDefaultView());
    rt.draw(m_UIContainer);
}
xy::StateID SailingState::stateID() const
{
	return States::Sailing;
}
