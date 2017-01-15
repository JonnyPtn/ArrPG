#pragma once

#include <xygine/State.hpp>
#include <xygine/ui/Container.hpp>
#include <xygine/Resource.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include "States.hpp"

class MainMenuState : public xy::State
{
public:
    MainMenuState(xy::StateStack& stack, xy::State::Context& context);
    ~MainMenuState();

    bool handleEvent(const sf::Event& evt) override;
    void handleMessage(const xy::Message&) override;
    bool update(float dt) override;
    void draw() override;
    xy::StateID stateID() const override;

private:
    sf::Texture         m_logoTexture;
    sf::Sprite          m_logoSprite;
    sf::Font            m_menuFont;
    sf::Text            m_newGameText;
    sf::Text            m_loadGameText;
    bool                m_canLoad;
};

