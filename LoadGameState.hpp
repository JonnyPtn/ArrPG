#pragma once

#include <xygine/State.hpp>
#include <xygine/ui/Container.hpp>
#include <xygine/ui/Selection.hpp>
#include <xygine/Resource.hpp>

#include "States.hpp"

class LoadGameState : public xy::State
{
public:
    LoadGameState(xy::StateStack & stack, xy::State::Context & context);

    // Inherited via State
    virtual bool handleEvent(const sf::Event & evt) override;
    virtual void handleMessage(const xy::Message &) override;
    virtual bool update(float dt) override;
    virtual void draw() override;
    virtual xy::StateID stateID() const override {return States::LoadGame;};
    
private:
    sf::RectangleShape  m_greyout;
    xy::UI::Container   m_UIContainer;
    xy::FontResource    m_fonts;
    xy::TextureResource m_textures;
    std::shared_ptr<xy::UI::Selection> m_fileSelection;
};

