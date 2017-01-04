#include "MainMenuState.hpp"
#include <xygine/App.hpp>
#include <xygine/util/Random.hpp>
#include <xygine/ui/Button.hpp>
#include <SFML/Window/Mouse.hpp>
#include "States.hpp"
#include "Messages.hpp"

MainMenuState::MainMenuState(xy::StateStack & stack, xy::State::Context & context)
    : xy::State(stack,context),
    m_UIContainer(context.appInstance.getMessageBus())
{
    //resources
    auto& buttonTexture = m_textures.get("Button.png");
    auto& buttonFont = m_UIFonts.get("Westmeath.ttf");

    auto xPos = context.renderWindow.getSize().x / 2;
    auto yIncrement = buttonTexture.getSize().y/3;
    auto yIndex(1);

    //create new world
    auto createWorldButton = xy::UI::create<xy::UI::Button>(buttonFont, buttonTexture);
    createWorldButton->setString("Create World");
    m_UIContainer.addControl(createWorldButton);
    createWorldButton->setAlignment(xy::UI::Alignment::Centre);
    createWorldButton->setPosition(xPos, yIncrement*yIndex++);
    createWorldButton->addCallback([this]()
    {
        requestStackPush(States::Sailing);
        //*msg = xy::Util::Random::value(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    });
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
