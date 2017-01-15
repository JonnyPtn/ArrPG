#include <SFML/Window/Mouse.hpp>

#include <xygine/App.hpp>
#include <xygine/ui/Button.hpp>

#include "PauseMenuState.hpp"
#include "States.hpp"


PauseMenuState::PauseMenuState(xy::StateStack& stack, xy::State::Context& context) :
    xy::State(stack,context),
    m_UIContainer(context.appInstance.getMessageBus())
{
    //save button
    auto saveButton = xy::UI::create<xy::UI::Button>(m_fonts.get("Westmeath.ttf"), m_textures.get());
    saveButton->setString("Save");
    saveButton->addCallback([this]()
    {
        //broadcast a save message
        requestStackPush(States::SaveGame);
        requestStackPop();
    });

    m_UIContainer.addControl(saveButton);

    m_UIContainer.setAlignment(xy::UI::Alignment::Centre);
    m_UIContainer.setPosition(context.defaultView.getCenter());
}


PauseMenuState::~PauseMenuState()
{
}

bool PauseMenuState::handleEvent(const sf::Event & evt)
{
    sf::Vector2f mPos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(getContext().renderWindow));
    m_UIContainer.handleEvent(evt,mPos);
    return false;
}

void PauseMenuState::handleMessage(const xy::Message &)
{
}

bool PauseMenuState::update(float dt)
{
    m_UIContainer.update(dt);
    return false;
}

void PauseMenuState::draw()
{
    auto& rt = getContext().renderWindow;
    rt.setView(rt.getDefaultView());
    rt.draw(m_UIContainer);
}

xy::StateID PauseMenuState::stateID() const
{
    return States::PauseMenu;
}
