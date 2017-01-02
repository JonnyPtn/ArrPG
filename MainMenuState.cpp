#include "MainMenuState.hpp"
#include <xygine/App.hpp>
#include <xygine/ui/Button.hpp>
#include <SFML/Window/Mouse.hpp>

MainMenuState::MainMenuState(xy::StateStack & stack, xy::State::Context & context)
    : xy::State(stack,context),
    m_UIContainer(context.appInstance.getMessageBus())
{
    auto& buttonTexture = m_textures.get("Button.png");
    auto& buttonFont = m_UIFonts.get("Westmeath.ttf");
    auto createWorldButton = xy::UI::create<xy::UI::Button>(buttonFont, buttonTexture);
    createWorldButton->setString("Create World");
    m_UIContainer.addControl(createWorldButton);
}

MainMenuState::~MainMenuState()
{
}

bool MainMenuState::handleEvent(const sf::Event & evt)
{
    auto mPos = sf::Mouse::getPosition(getContext().renderWindow);
    m_UIContainer.handleEvent(evt, sf::Vector2f(mPos));
    return false;
}

void MainMenuState::handleMessage(const xy::Message &)
{
}

bool MainMenuState::update(float dt)
{
    m_UIContainer.update(dt);
    return false;
}

void MainMenuState::draw()
{
    auto& rt = getContext().renderWindow;
    rt.draw(m_UIContainer);
}

xy::StateID MainMenuState::stateID() const
{
    return States::MainMenu;
}
