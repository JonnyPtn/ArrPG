#pragma once

#include <xygine/State.hpp>
#include <xygine/ui/Container.hpp>
#include <xygine/Resource.hpp>
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
    xy::UI::Container   m_UIContainer;
    xy::FontResource    m_UIFonts;
    xy::TextureResource m_textures;
};

