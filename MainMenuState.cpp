#include "MainMenuState.hpp"
#include <xygine/App.hpp>
#include <xygine/util/Random.hpp>
#include <xygine/ui/Button.hpp>
#include <SFML/Window/Mouse.hpp>
#include "States.hpp"
#include "Messages.hpp"
#include <SFML/Window/Event.hpp>

MainMenuState::MainMenuState(xy::StateStack & stack, xy::State::Context & context)
    : xy::State(stack,context),
    m_canLoad(true)
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

    //disable load game if saves folder is empty
    auto files = xy::FileSystem::listFiles("saves/");
    if (files.empty())
    {
        m_canLoad = false;
        m_loadGameText.setColor({ 128,128,128,128 });
    }
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
            //go straight to playing
            requestStackPop();
            requestStackPush(States::Playing);
        }
        else if (m_canLoad && m_loadGameText.getGlobalBounds().contains(mPos))
        {
            //go to playing but load a game first
            requestStackPop();
            requestStackPush(States::Playing);
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
