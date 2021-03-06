#pragma once

#include <xygine/State.hpp>
#include <xygine/ui/Container.hpp>
#include <xygine/ui/TextBox.hpp>
#include <xygine/ui/Selection.hpp>
#include <xygine/Resource.hpp>

#include "States.hpp"

class SaveGameState : public xy::State
{
public:
    SaveGameState(xy::StateStack & stack, xy::State::Context & context);

    // Inherited via State
    virtual bool handleEvent(const sf::Event & evt) override;
    virtual void handleMessage(const xy::Message &) override;
    virtual bool update(float dt) override;
    virtual void draw() override;
    virtual xy::StateID stateID() const override {return States::LoadGame;};
    
private:
    xy::UI::Container                   m_UIContainer;
    xy::FontResource                    m_fonts;
    xy::TextureResource                 m_textures;
    std::shared_ptr<xy::UI::TextBox>    m_inputBox;
    std::shared_ptr<xy::UI::Selection>  m_currentSaveSelection;
};

