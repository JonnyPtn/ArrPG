#include "WindController.hpp"
#include <xygine/App.hpp>
#include <xygine/imgui/imgui.h>
#include <xygine/util/Vector.hpp>

WindController::WindController(xy::MessageBus& mb) :
	xy::Component(mb, this),
	m_windVelocity(0.f, 0.f)
{
	xy::App::addUserWindow([&]()
	{
		static float direction(180.f), strength(0.f);
		if (ImGui::SliderFloat("direction (degrees)", &direction, 0, 360))
		{
			m_windVelocity = xy::Util::Vector::rotate({ 0.f,-strength }, direction);
		}
		if (ImGui::SliderFloat("strength", &strength, 0, 100))
		{
			m_windVelocity = xy::Util::Vector::rotate({ 0.f,-strength }, direction);
		}
	});
}


WindController::~WindController()
{
}

void WindController::entityUpdate(xy::Entity & entity, float dt)
{
}

sf::Vector2f WindController::getWindVelocity() const
{
	return m_windVelocity;
}
