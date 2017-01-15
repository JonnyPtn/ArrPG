#include "SailingState.hpp"
#include "States.hpp"
#include <xygine/Entity.hpp>
#include <xygine/components/SfDrawableComponent.hpp>
#include <xygine/StateStack.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <xygine/App.hpp>
#include "PlayerController.hpp"
#include <xygine/physics/RigidBody.hpp>
#include <xygine/physics/CollisionRectangleShape.hpp>
#include <xygine/physics/JointFriction.hpp>
#include <SFML/Window/Event.hpp>
#include "WorldController.hpp"
#include <xygine/Resource.hpp>
#include <xygine/util/Random.hpp>
#include "IslandComponent.hpp"
#include "Messages.hpp"
#include <xygine/postprocess/OldSchool.hpp>
#include <xygine/components/Camera.hpp>
#include "InventoryComponent.hpp"
#include "InventoryUI.hpp"

SailingState::SailingState(xy::StateStack & stack, xy::State::Context& context)
    : xy::State(stack, context),
    m_messageBus(context.appInstance.getMessageBus()),
    m_scene(m_messageBus),
    m_physicsWorld(m_messageBus),
    m_UIContainer(m_messageBus),
    m_snapToNorth(false)
{
    xy::App::setClearColour({ 0,41,58 });
  
	//no gravity because top-down
	m_physicsWorld.setGravity({ 0.f,0.f });

    //add the world Entity
    auto world = xy::Entity::create(m_messageBus);
    auto worldController = xy::Component::create<WorldController>(m_messageBus);
    auto wc = world->addComponent(worldController);
    m_world = m_scene.addEntity(world, xy::Scene::Layer::BackRear);

    //add the player Entity
    auto player = xy::Entity::create(m_messageBus);
    auto playerComponent = xy::Component::create<PlayerController>(m_messageBus,*wc);
    auto bounds = player->globalBounds();
    player->setOrigin(bounds.width / 2, bounds.height / 2);
    player->addComponent(playerComponent);

    //player inventory
    auto inventory = xy::Component::create<InventoryComponent>(m_messageBus, 16);
    m_playerInventory = player->addComponent(inventory);


    //player camera
    auto cam = xy::Component::create<xy::Camera>(m_messageBus, m_scene.getView());
    cam->setZoom(1.f);
    m_playerCam = player->addComponent(cam);

    m_player = m_scene.addEntity(player, xy::Scene::Layer::FrontFront);

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

    //inventory display
    m_inventory = xy::UI::create<InventoryUI>(*m_playerInventory);
    m_inventory->setPosition(50, 500);
    m_UIContainer.addControl(m_inventory);
}

SailingState::~SailingState()
{
}

bool SailingState::handleEvent(const sf::Event & evt)
{
    switch (evt.type)
    {
        //zoom view on mouse wheel scroll
    case sf::Event::MouseWheelScrolled:
    {
        auto view = m_scene.getView();
        m_playerCam->setZoom(1.f + evt.mouseWheelScroll.delta*0.2f);
        break;
    }

    case sf::Event::KeyPressed:
    {
        switch (evt.key.code)
        {
        case sf::Keyboard::Escape:
        {
            //pause menu
            requestStackPush(States::PauseMenu);
            return true;
        }
        }
    }
    }

    //update UI
    auto mousePos = sf::Mouse::getPosition(getContext().renderWindow);
    m_UIContainer.handleEvent(evt, sf::Vector2f(mousePos));
    return false;
}

void SailingState::handleMessage(const xy::Message &message)
{
	m_scene.handleMessage(message);

    //check for inventory changes to update UI
    if (message.id == Messages::INVENTORY_CHANGE)
    {
        m_inventory->update();
    }
}

bool SailingState::update(float dt)
{
    m_scene.update(dt);
    auto boatPos = m_player->getWorldPosition(); 
    if (m_snapToNorth)
    {
        m_compass->setRotation(-m_player->getRotation());
        //view.setRotation(m_player->getRotation());
    }
    else
    {
        m_compass->setRotation(0);
    }

    //update co-ordinates
    m_xPosDisplay->setString(std::to_string(boatPos.x/100.f));
    m_yPosDisplay->setString(std::to_string(boatPos.y/100.f));

    //and the UI
    m_UIContainer.update(dt);

    //check for player collisions
	return false;
}

void SailingState::draw()
{
    auto& rt = getContext().renderWindow;

    m_scene.setActiveCamera(m_playerCam);
    rt.draw(m_scene);
   // rt.draw(m_physicsWorld);

    rt.setView(rt.getDefaultView());
    rt.draw(m_UIContainer);
}
xy::StateID SailingState::stateID() const
{
    return States::Playing;
}
