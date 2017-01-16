#include "SeaDrawable.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include <xygine/Entity.hpp>
#include <xygine/Scene.hpp>

namespace
{
    const std::string seaShader =
        R"(

// Found this on GLSL sandbox. I really liked it, changed a few things and made it tileable.
// :)
// by David Hoskins.


// Water turbulence effect by joltz0r 2013-07-04, improved 2013-07-07

#define TAU 6.28318530718
#define MAX_ITER 5

uniform float iGlobalTime;
uniform vec2 iResolution;
uniform vec2 offset;

void main(void)
{
  float time = iGlobalTime * .5+23.0;
    // uv should be the 0-1 uv of texture...
	vec2 uv = ((gl_FragCoord.xy + offset.xy)/ iResolution.xy);
    
#ifdef SHOW_TILING
	vec2 p = mod(uv*TAU*2.0, TAU)-250.0;
#else
    vec2 p = mod(uv*TAU, TAU)-250.0;
#endif
	vec2 i = vec2(p);
	float c = 1.0;
	float inten = .005;

	for (int n = 0; n < MAX_ITER; n++) 
	{
		float t = time * (1.0 - (3.5 / float(n+1)));
		i = p + vec2(cos(t - i.x) + sin(t + i.y), sin(t - i.y) + cos(t + i.x));
		c += 1.0/length(vec2(p.x / (sin(i.x+t)/inten),p.y / (cos(i.y+t)/inten)));
	}
	c /= float(MAX_ITER);
	c = 1.17-pow(c, 1.4);
	vec3 colour = vec3(pow(abs(c), 8.0));
    colour = clamp(colour + vec3(0.0, 0.35, 0.5), 0.0, 1.0);
    

	#ifdef SHOW_TILING
	// Flash tile borders...
	vec2 pixel = 2.0 / iResolution.xy;
	uv *= 2.0;

	float f = floor(mod(iGlobalTime*.5, 2.0)); 	// Flash value.
	vec2 first = step(pixel, uv) * f;		   	// Rule out first screen pixels and flash.
	uv  = step(fract(uv), pixel);				// Add one line of pixels per tile.
	colour = mix(colour, vec3(1.0, 1.0, 0.0), (uv.x + uv.y) * first.x * first.y); // Yellow line
	
	#endif
	gl_FragColor = vec4(colour, 1.0);
})";
}


SeaDrawable::SeaDrawable(xy::MessageBus& mb) :
    xy::Component(mb, this)
{
    //load the shader
    m_shader.loadFromMemory(seaShader, sf::Shader::Fragment);
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
    m_shader.setUniform("iResolution", m_shape.getSize());
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
