#pragma once

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <xygine/components/Component.hpp>

class SeaDrawable : public xy::Component, public sf::Drawable
{
public:
    SeaDrawable(xy::MessageBus& mb);
    ~SeaDrawable();

    // Inherited via Component
    virtual Type type() const override;

    void onDelayedStart(xy::Entity& ent) override;

    virtual void entityUpdate(xy::Entity &, float) override;
    sf::FloatRect globalBounds() const override;

private:
    sf::RectangleShape  m_shape;
    sf::Shader          m_shader;
    sf::Texture         m_seaNoiseTexture;
    sf::Texture         m_sandTexture;
    sf::Clock           m_clock;
    float               m_seaLevel = 0.9; //range 0.0-2.0
    sf::Vector3f        m_sunDirection;

    // Inherited via Drawable
    virtual void draw(sf::RenderTarget & target, sf::RenderStates states) const override;
};

