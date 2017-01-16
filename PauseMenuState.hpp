#pragma once
#include <xygine/State.hpp>
#include <xygine/ui/Container.hpp>
#include <xygine/Resource.hpp>

class PauseMenuState : public xy::State
{
public:
    PauseMenuState(xy::StateStack& stack, xy::State::Context& context);
    ~PauseMenuState();

    // Inherited via State
    virtual bool handleEvent(const sf::Event & evt) override;
    virtual void handleMessage(const xy::Message &) override;
    virtual bool update(float dt) override;
    virtual void draw() override;
    virtual xy::StateID stateID() const override;

private:
    xy::UI::Container   m_UIContainer;
    xy::FontResource    m_fonts;
    xy::TextureResource m_textures;
    sf::RectangleShape  m_greyout;
   
};

