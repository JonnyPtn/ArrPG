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
#include "FastNoise.h"
#include "Messages.hpp"
#include <stack>
constexpr float maxSize(8000.f);
constexpr float minSize(4000.f);

float IslandComponent::m_seaLevel(0.5f);

IslandComponent::IslandComponent(xy::MessageBus& mb, int seed) :
    m_seed(seed),
    m_currentDiagram(nullptr),
    m_entity(nullptr),
    xy::Component(mb, this),
    vGen(),
    m_noise()
{
    m_noise.SetNoiseType(FastNoise::NoiseType::SimplexFractal);
    auto size = xy::Util::Random::value(minSize, maxSize);
    m_bounds = {0.f, 0.f, size, size};

    //handle sea level changes
    xy::Component::MessageHandler handler;
    handler.id = Messages::SEA_LEVEL_CHANGED;
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
    if(!m_entity)
        m_entity = &ent;

    getRandomSites(2048);

    m_noise = FastNoise();
    m_noise.SetNoiseType(FastNoise::NoiseType::SimplexFractal);
    create(vGen.compute(m_sites,m_bounds));
}

template<typename T>
bool IslandComponent::canReachOcean(const Cell<T>& cell, std::unordered_set<const Cell<float>*>& checkedCells)
{
    //if we're an edge cell then return true
    if (cell.closeMe)
        return true;

    //if we've already been checked move on
    if (checkedCells.find(&cell) != checkedCells.end())
        return false;

    checkedCells.insert(&cell);

    //if we aren't water then return false
    if (m_waterCells.find(&cell) == m_waterCells.end())
        return false;

    //otherwise check our neighbours
    for (auto& n : cell.getNeighbors())
    {
        if (canReachOcean(*n,checkedCells))
            return true;
    }
    return false;
}

bool IslandComponent::isLand(const sf::Vector2f& position)
{
    //convert to island local co-ordinates
    auto localPos = m_entity->getTransform().getInverse().transformPoint(position);

    for (auto cell : m_currentDiagram->cells)
    {
        //first check the bounding box
        auto bounds = cell->getBoundingBox();
        if (bounds.contains(sf::Vector2<float>(localPos)))
        {
            //then check the point detection
            if (cell->pointIntersection(localPos.x, localPos.y) == 1)
                return isLand(cell);
       }
    }
    return false;
}

bool IslandComponent::isLand(const Cell<float>* cell)
{
    auto edges = cell->halfEdges;

    //check the ratio of corners which are above sea level
    int cornerCount(0), landCount(0);
    for (auto e : edges)
    {
        if (getHeight(*e->startPoint()) > m_seaLevel)
        {
            landCount++;
        }
        cornerCount++;
    }

    //set ratio of corners which have to be land for it to be a land cell
    auto cornerRatio = 0.2;

    return (landCount / cornerCount) > cornerRatio;
}

template<typename T>
void IslandComponent::create(Diagram<T>* diagram)
{
   if(m_currentDiagram && m_currentDiagram != diagram)
        delete m_currentDiagram;
    m_currentDiagram = diagram;

    //clear all current cells
    m_cellTypes.clear();

    //add coast and ocean
    addOceanAndCoastCells();

    //add land and lakes
    addLandAndLakeCells();

    //clear
    m_landPolys.clear();

    //create land polys
    for (auto& cell : m_currentDiagram->cells)
    {
        //adjust the colour based on type, ignoring ocean tiles
        sf::Color cellColour;
        auto cellType = m_cellTypes.find(cell);
        if (cellType != m_cellTypes.end())
        {
            switch (cellType->second)
            {
            case CellType::COAST:
                cellColour = { 239, 221, 111 }; //sandy colour
                break;

            case CellType::LAND:
                cellColour = { 0,100,0 };
                break;

            case CellType::LAKE:
                cellColour = { 105,193,201 }; //lake-ier blue than the ocean
                break;
            }

            //add polys for everything except ocean
            if (cellType->second != CellType::OCEAN)
            {
                //go through all the half edges
                for (auto h : cell->halfEdges)
                {
                    //add a tri using start, end and site point
                    m_landPolys.push_back({ sf::Vector2f(h->startPoint()->x,h->startPoint()->y),cellColour });
                    m_landPolys.push_back({ sf::Vector2f(h->endPoint()->x, h->endPoint()->y),cellColour });
                    m_landPolys.push_back({ sf::Vector2f(cell->site.p.x, cell->site.p.y),cellColour });
                }
            }
        }
    }
}

float IslandComponent::getHeight(sf::Vector2f pos)
{
    sf::Vector2f centre;
    centre.x = localBounds().width / 2;
    centre.y = localBounds().height / 2;

    //get the distance between the centre and the position
    auto d = pos - centre;
    auto dv = xy::Util::Vector::length(sf::Vector2f(d.x, d.y));

    //make it a factor of the max distance
    auto df = 1 - (dv / (m_bounds.width));

    //if the noise value factored by the distance is over sea level, it's land
    auto n = (m_noise.GetSimplexFractal(pos.x, pos.y) + 1) / 2;

    return n;
}

void IslandComponent::getRandomSites(int count)
{
    m_sites.resize(count);
    for (auto& site : m_sites)
    {
        site.x = xy::Util::Random::value(0.f, m_bounds.width);
        site.y = xy::Util::Random::value(0.f, m_bounds.height);
    }

    //sort them
    std::sort(m_sites.begin(), m_sites.end(), [](const sf::Vector2<float>& s1, const sf::Vector2<float>& s2)
    {
        if (s1.y < s2.y)
            return true;
        if (s1.y == s2.y && s1.x < s2.x)
            return true;
        return false;
    });
}

void IslandComponent::addOceanAndCoastCells()
{
    //gather all the edge cells
    std::list<Cell<float>*> edgeCells;
    for (auto c : m_currentDiagram->cells)
    {
        if (c->closeMe)
            edgeCells.push_back(c);
    }

    for (auto c : edgeCells)
    {
        //if it hasn't already been explored
        if (m_cellTypes.find(c) == m_cellTypes.end())
        {
            //stack of explored cells, starting at this edge
            std::stack<Cell<float>*> explorePath;
            explorePath.push(c);
            do
            {
                auto cell = explorePath.top();
                bool found = false;
                //explore an unexplored neighbour
                for (auto n : cell->getNeighbors())
                {
                    if (m_cellTypes.find(n) == m_cellTypes.end())
                    {
                        //check for edge cell
                        if (n->closeMe)
                        {
                            m_cellTypes[n] = CellType::OCEAN;
                            explorePath.push(n);
                            found = true;
                            break;
                        }
                        else if (isLand(n))
                        {
                            m_cellTypes[n] = CellType::COAST;
                        }
                        else
                        {
                            m_cellTypes[n] = CellType::OCEAN;
                            explorePath.push(n);
                            found = true;
                            break;
                        }
                    }
                }
                if (!found)
                {
                    //haven't found an unexplored neighbour, go back up the stack
                    explorePath.pop();
                }
            } while (!explorePath.empty());
        }
    }
}

void IslandComponent::addLandAndLakeCells()
{
    //go through all cells
    for (auto c : m_currentDiagram->cells)
    {
        //first check it's not already assigned
        if (m_cellTypes.find(c) == m_cellTypes.end())
        {
            //if it's above water, it's land, otherwise it's lake
            if (isLand(c))
            {
                m_cellTypes[c] = CellType::LAND;
            }
            else
            {
                m_cellTypes[c] = CellType::LAKE;
            }
        }
    }
}

void IslandComponent::draw(sf::RenderTarget & target, sf::RenderStates states) const
{
    states.transform *= getTransform();
    target.draw(m_landPolys.data(),m_landPolys.size(),sf::PrimitiveType::Triangles, states);

    //debug
    //target.draw(*m_currentDiagram, states);
}

void IslandComponent::updateSeaLevel(float seaLevel)
{
    m_seaLevel = seaLevel;
    create(m_currentDiagram);
}

sf::FloatRect IslandComponent::localBounds() const
{
    return sf::FloatRect(m_bounds);
}

sf::FloatRect IslandComponent::globalBounds() const
{
    return getTransform().transformRect(sf::FloatRect(m_bounds));
}
