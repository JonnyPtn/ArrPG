#include "MainMenuState.hpp"
#include <xygine/App.hpp>
#include <xygine/util/Random.hpp>
#include <xygine/ui/Button.hpp>
#include <SFML/Window/Mouse.hpp>
#include "States.hpp"
#include "Messages.hpp"
#include <SFML/Window/Event.hpp>

MainMenuState::MainMenuState(xy::StateStack & stack, xy::State::Context & context)
    : xy::State(stack,context)
{
    xy::App::setClearColour(sf::Color::White);
    //logo
    m_logoTexture.loadFromFile("logo.png");
    m_logoSprite.setTexture(m_logoTexture);
    m_logoSprite.setPosition({ static_cast<float>(context.renderWindow.getSize().x )/ 2,0 });
    m_logoSprite.setOrigin({ static_cast<float>(m_logoTexture.getSize().x )/ 2,0 });
    m_logoSprite.setScale({ 3.f,3.f });

    //font
    m_menuFont.loadFromFile("Westmeath.ttf");

    //new game
    m_newGameText.setFont(m_menuFont);
    m_newGameText.setString("New Game");
    auto lb = m_newGameText.getLocalBounds();
    auto gb = m_newGameText.getGlobalBounds();
    m_newGameText.setOrigin(gb.width / 2, lb.top + gb.height / 2);
    m_newGameText.setPosition(context.renderWindow.getSize().x / 2, context.renderWindow.getSize().y - 100);
    m_newGameText.setFillColor(sf::Color::Black);

    //load game
    m_loadGameText.setFont(m_menuFont);
    m_loadGameText.setString("Load Game");
    lb = m_loadGameText.getLocalBounds();
    gb = m_loadGameText.getGlobalBounds();
    m_loadGameText.setOrigin(gb.width / 2, lb.top + gb.height / 2);
    m_loadGameText.setPosition(context.renderWindow.getSize().x / 2, context.renderWindow.getSize().y - 50);
    m_loadGameText.setFillColor(sf::Color::Black);
}

MainMenuState::~MainMenuState()
{
}

bool MainMenuState::handleEvent(const sf::Event & evt)
{
    sf::Vector2f mPos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(getContext().renderWindow));

    if (evt.type == sf::Event::MouseButtonPressed)
    {
        if (m_newGameText.getGlobalBounds().contains(mPos))
        {
            //start a new game
            requestStackPop();
            requestStackPush(States::NewGame);
        }
        else if (m_loadGameText.getGlobalBounds().contains(mPos))
        {
            //start a new game
            requestStackPop();
            requestStackPush(States::LoadGame);
        }
    }
    return false;
}

void MainMenuState::handleMessage(const xy::Message &)
{
}

bool MainMenuState::update(float dt)
{
    return false;
}

void MainMenuState::draw()
{
    auto& rt = getContext().renderWindow;
    rt.draw(m_logoSprite);
    rt.draw(m_loadGameText);
    rt.draw(m_newGameText);
}

xy::StateID MainMenuState::stateID() const
{
    return States::MainMenu;
}
