#include "IslandComponent.hpp"

#include <xygine/Scene.hpp>
#include <xygine/components/SfDrawableComponent.hpp>
#include <xygine/physics/CollisionPolygonShape.hpp>
#include <xygine/QuadTreeNode.hpp>
#include <xygine/physics/RigidBody.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <xygine/util/Random.hpp>
#include <xygine/util/Vector.hpp>
#include <xygine/App.hpp>
#include <xygine/imgui/imgui.h>
#include <FastNoise/FastNoise.h>
#include "Messages.hpp"

constexpr float maxSize(8000.f);
constexpr float minSize(4000.f);


IslandComponent::IslandComponent(xy::MessageBus& mb, int seed) :
    m_seed(seed),
    m_sites(500),
    m_currentDiagram(nullptr),
	xy::Component(mb,this)
{
    m_bounds = { 0.0,xy::Util::Random::value(minSize, maxSize)
        , xy::Util::Random::value(minSize, maxSize),0.0 };

    //handle sea level changes
    xy::Component::MessageHandler handler;
    handler.id = Messages::SeaLevelChanged;
    handler.action = [this](xy::Component* c, const xy::Message& msg)
    {
        auto& data = msg.getData<float>();
        updateSeaLevel(data);
    };
    addMessageHandler(handler);
}

IslandComponent::~IslandComponent()
{
}

void IslandComponent::onStart(xy::Entity & ent)
{
    getRandomSites(1024);

    m_noise = FastNoise();
    create(vGen.compute(m_sites,m_bounds));
}

bool    IslandComponent::isLand(Cell* cell)
{
    Point2 centre;
    centre.x = localBounds().width / 2;
    centre.y = localBounds().height / 2;

    //get the distance between the centre and the cell
    auto d = cell->site.p - centre;
    auto dv = xy::Util::Vector::length(sf::Vector2f( d.x,d.y ));

    //make it a factor of the max distance
    auto df = 1 - (dv / (m_bounds.xR - m_bounds.xL));

    //if the noise value factored by the distance is over sea level, it's land
    auto n = (m_noise.GetSimplexFractal(cell->site.p.x, cell->site.p.y)+1)/2;
  
    return df* n > m_seaLevel;
}

void IslandComponent::create(Diagram* diagram)
{
   if(m_currentDiagram && m_currentDiagram != diagram)
        delete m_currentDiagram;
    m_currentDiagram = diagram;


    //clear
    m_voronoiLines.clear();
    m_points.clear();
    m_triLines.clear();
    m_landPolys.clear();

    //cell drawables

    for (auto& cell : m_currentDiagram->cells)
    {
        if (isLand(cell))
        {
            //draw the centre of the cell
            m_points.push_back({ sf::Vector2f(cell->site.p.x, cell->site.p.y), sf::Color::Red });

            //go through all the half edges
            for (auto h : cell->halfEdges)
            {
                //add a tri using start, end and site point
                m_landPolys.push_back({ sf::Vector2f(h->startPoint()->x,h->startPoint()->y),sf::Color::Green });
                m_landPolys.push_back({ sf::Vector2f(h->endPoint()->x, h->endPoint()->y),sf::Color::Green });
                m_landPolys.push_back({ sf::Vector2f(cell->site.p.x, cell->site.p.y),sf::Color::Green });

                //voroi lines
                m_voronoiLines.push_back({ sf::Vector2f(h->endPoint()->x,h->endPoint()->y),sf::Color::Cyan });
                m_voronoiLines.push_back({ sf::Vector2f(h->startPoint()->x,h->startPoint()->y),sf::Color::Cyan });
            }
        }
    }
}

void IslandComponent::getRandomSites(int count)
{
    m_sites.resize(count);
    for (auto site : m_sites)
    {
        site.x = xy::Util::Random::value(0, m_bounds.xR);
        site.y = xy::Util::Random::value(0, m_bounds.yB);
    }

    //sort them
    std::sort(m_sites.begin(), m_sites.end(), [](const Point2& s1, const Point2& s2)
    {
        if (s1.y < s2.y)
            return true;
        if (s1.y == s2.y && s1.x < s2.x)
            return true;

        return false;
    });

    //more accurate types
    double maxX(0.), maxY(0.);
    for (int i = 0; i<m_sites.size(); i++)
    {
        m_sites[i].x = xy::Util::Random::value(0.f, m_bounds.xR);
        m_sites[i].y = xy::Util::Random::value(0.f, m_bounds.yB);
        if (m_sites[i].x > maxX)
            maxX = m_sites[i].x;
        if (m_sites[i].y > maxY)
            maxY = m_sites[i].y;
    }
    m_bounds = { 0,maxX,maxY,0 };
}

void IslandComponent::draw(sf::RenderTarget & target, sf::RenderStates states) const
{
    states.transform *= getTransform();
    target.draw(m_landPolys.data(), m_landPolys.size(), sf::PrimitiveType::Triangles, states);
    target.draw(m_voronoiLines.data(), m_voronoiLines.size(), sf::PrimitiveType::Lines, states);
    target.draw(m_points.data(), m_points.size(), sf::PrimitiveType::Points, states);
}

void IslandComponent::updateSeaLevel(float seaLevel)
{
    m_seaLevel = seaLevel;

    //clear land polys and regenerate
    m_landPolys.clear();
    m_voronoiLines.clear();
     for (auto& cell : m_currentDiagram->cells)
    {
        if (isLand(cell))
        {
            //draw the centre of the cell
            m_points.push_back({ sf::Vector2f(cell->site.p.x, cell->site.p.y), sf::Color::Red });

            //go through all the half edges
            for (auto h : cell->halfEdges)
            {
                //add a tri using start, end and site point
                m_landPolys.push_back({ sf::Vector2f(h->startPoint()->x,h->startPoint()->y),sf::Color::Green });
                m_landPolys.push_back({ sf::Vector2f(h->endPoint()->x, h->endPoint()->y),sf::Color::Green });
                m_landPolys.push_back({ sf::Vector2f(cell->site.p.x, cell->site.p.y),sf::Color::Green });

                //voroi lines
                m_voronoiLines.push_back({ sf::Vector2f(h->endPoint()->x,h->endPoint()->y),sf::Color::Cyan });
                m_voronoiLines.push_back({ sf::Vector2f(h->startPoint()->x,h->startPoint()->y),sf::Color::Cyan });
            }
        }
    }
}

sf::FloatRect IslandComponent::localBounds() const
{
	return sf::FloatRect(m_bounds.xL,m_bounds.yT,m_bounds.xR - m_bounds.xL,m_bounds.yB - m_bounds.yT);
}

sf::FloatRect IslandComponent::globalBounds() const
{
    return getTransform().transformRect(sf::FloatRect(m_bounds.xL, m_bounds.yT, m_bounds.xR - m_bounds.xL, m_bounds.yB - m_bounds.yT));
}
