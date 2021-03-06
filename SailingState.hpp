#pragma once
#include <xygine/State.hpp>
#include <xygine/StateStack.hpp>
#include <xygine/MessageBus.hpp>
#include <xygine/Scene.hpp>
#include <xygine/physics/World.hpp>
#include <xygine/Resource.hpp>
#include <xygine/ui/Container.hpp>
#include <xygine/ui/Label.hpp>
#include <xygine/ui/Button.hpp>
#include "WorldController.hpp"
#include "InventoryComponent.hpp"
#include "InventoryUI.hpp"

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
	xy::Entity*				m_player;
    xy::Entity*             m_world;
	sf::RectangleShape*		m_seaShape;
	xy::TextureResource		m_textures;
    std::string             m_saveFilePath;
    xy::Camera*             m_playerCam;
    InventoryComponent*     m_playerInventory;

    //UI
    xy::UI::Container               m_UIContainer;
    xy::FontResource                m_UIFonts;
    std::shared_ptr<xy::UI::Label>  m_xPosDisplay;
    std::shared_ptr<xy::UI::Label>  m_yPosDisplay;
    std::shared_ptr<xy::UI::Button> m_compass;
    std::shared_ptr<InventoryUI>    m_inventory;
    bool                            m_snapToNorth;
};

