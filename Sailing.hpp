#pragma once

#include <xygine\App.hpp>
#include <xygine\StateStack.hpp>

class Sailing : public xy::App
{
public:
	Sailing();
	void handleEvent(const sf::Event&) override;
	void handleMessage(const xy::Message&) override;

	void registerStates() override;
	void updateApp(float dt) override;
	void draw() override;

private:
	xy::StateStack m_stateStack;
};

