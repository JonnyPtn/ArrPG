#include "Sailing.hpp"
#include "SailingState.hpp"
#include "MainMenuState.hpp"
#include "States.hpp"

Sailing::Sailing() :
    xy::App(), // for aa
	m_stateStack({ getRenderWindow(),*this }) 
{
	registerStates(); 
}

void Sailing::handleEvent(const sf::Event &evt)
{
	m_stateStack.handleEvent(evt);
}

void Sailing::handleMessage(const xy::Message & message)
{
	m_stateStack.handleMessage(message);
}

void Sailing::registerStates()
{
    m_stateStack.registerState<SailingState>(States::NewGame,"");
    m_stateStack.registerState<SailingState>(States::LoadGame, "world.boop");
    m_stateStack.registerState<MainMenuState>(States::MainMenu);

	//first state
	m_stateStack.pushState(States::MainMenu);
}

void Sailing::updateApp(float dt)
{
	m_stateStack.update(dt);
}

void Sailing::draw()
{
	m_stateStack.draw();
}
