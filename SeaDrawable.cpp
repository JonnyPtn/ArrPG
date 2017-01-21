#include "SeaDrawable.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <xygine/Entity.hpp>
#include <xygine/Scene.hpp>
#include <xygine/App.hpp>
#include <xygine/imgui/imgui_sfml.h>
#include <xygine/imgui/imgui.h>
#include <chrono>
#include "Messages.hpp"
#include "FastNoise.h"
#include "WorldController.hpp"
#include <xygine/util/Math.hpp>
#include <xygine/util/Vector.hpp>

SeaDrawable::SeaDrawable(xy::MessageBus& mb) :
    xy::Component(mb, this)
{
    //load the shader
    m_shader.loadFromFile("assets/shaders/sea.frag", sf::Shader::Fragment);
    m_clock.restart();
}


SeaDrawable::~SeaDrawable()
{
}

xy::Component::Type SeaDrawable::type() const
{
    return xy::Component::Type::Drawable;
}

void SeaDrawable::onDelayedStart(xy::Entity & ent)
{
    auto size = ent.getScene()->getView().getSize();
    m_shape.setSize({ size.x, size.y });
    m_shader.setUniform("iResolution", sf::Vector2f(m_shape.getSize().x, m_shape.getSize().y));

    //sand texture for shader
    m_sandTexture.loadFromFile("assets/terrain/sand.png");
    m_shader.setUniform("terrainTex", m_sandTexture);

    //white noise texture for shader
    FastNoise noise;
    auto now = std::chrono::system_clock::now().time_since_epoch().count();
    noise.SetSeed(now);
    noise.SetFrequency(1.0);
    noise.SetNoiseType(FastNoise::Simplex);
    sf::Image noiseImage;
    noiseImage.create(256, 256);
    for (int x = 0; x < noiseImage.getSize().x; x++)
    {
        for (int y = 0; y < noiseImage.getSize().y; y++)
        {
            auto n = (noise.GetValue(x, y) +1)/2;
            sf::Uint8 nc = 255 * n; //fraction as uint8 for colour
            noiseImage.setPixel(x, y, { nc,nc,nc });
        }
    }
    m_seaNoiseTexture.loadFromImage(noiseImage);
    m_shader.setUniform("noiseTex", m_seaNoiseTexture);

    //handle sea level changes
    xy::Component::MessageHandler handler;
    handler.id = Messages::SEA_LEVEL_CHANGED;
    handler.action = [this](xy::Component* c, const xy::Message& msg)
    {
        auto& data = msg.getData<float>();
        m_seaLevel = data;
        //needs to be in the range 0-2.0 for the shader
        m_shader.setParameter("waterLevel", m_seaLevel*2);
    };
    addMessageHandler(handler);

    //debug
    xy::App::addUserWindow([this]()
    {
        sf::Sprite noiseSprite(m_seaNoiseTexture);
        ImGui::Image(noiseSprite);
        if (ImGui::SliderFloat("Sea Level", &m_seaLevel, 0.f, 2.f))
        {
            m_shader.setParameter("waterLevel", m_seaLevel);
        }

        //sun position
        if (ImGui::SliderFloat("sunX", &m_sunDirection.x, -5.f, 5.f))
        {
            m_shader.setParameter("light", m_sunDirection);
        }
        if (ImGui::SliderFloat("sunY", &m_sunDirection.y, -5.f, 5.f))
        {
            m_shader.setParameter("light", m_sunDirection);
        }
        if (ImGui::SliderFloat("sunZ", &m_sunDirection.z, -5.f, 5.f))
        {
            m_shader.setParameter("light", m_sunDirection);
        }
    });

}

void SeaDrawable::entityUpdate(xy::Entity & ent, float)
{
    m_shader.setUniform("iGlobalTime", m_clock.getElapsedTime().asSeconds());
    auto pos = ent.getPosition();
    pos.y = -pos.y; //flip y axis for shader
    m_shader.setUniform("offset", pos);

    //get the position of the camera
    auto va = ent.getScene()->getVisibleArea();
    ent.setPosition(va.left, va.top);

    //get the time of day
    auto world = ent.getComponent<WorldController>();
    if (world)
    {
        auto time = world->getWorldTimeOfDay();
        
        //get a 2d angle and convert to 3d
        auto sunDir = xy::Util::Vector::rotate({ 0,1 },time*360);
        m_sunDirection = { sunDir.x,0.55f,(-sunDir.y)*3.f };

        m_shader.setParameter("light", m_sunDirection);
    }
}

sf::FloatRect SeaDrawable::globalBounds() const
{
    return m_shape.getGlobalBounds();
}

void SeaDrawable::draw(sf::RenderTarget & target, sf::RenderStates states) const
{
    states.shader = &m_shader;
    target.draw(m_shape, states);
}
