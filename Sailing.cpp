#include "Sailing.hpp"
#include "SailingState.hpp"
#include "MainMenuState.hpp"
#include "States.hpp"

Sailing::Sailing() :
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
    m_stateStack.registerState<SailingState>(States::Sailing);
    m_stateStack.registerState<MainMenuState>(States::MainMenu);

	//first state
	m_stateStack.pushState(States::Sailing);
}

void Sailing::updateApp(float dt)
{
	m_stateStack.update(dt);
}

void Sailing::draw()
{
	m_stateStack.draw();
}
