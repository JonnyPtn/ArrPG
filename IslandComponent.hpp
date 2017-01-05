#pragma once

#include <xygine/components/Component.hpp>
#include <xygine/Scene.hpp>
#include <xygine/Entity.hpp>
#include <VoronoiDiagramGenerator.h>
#include <SFML/Graphics/CircleShape.hpp>
#include "FastNoise/FastNoise.h"

class IslandComponent : public xy::Component, public sf::Drawable, public sf::Transformable
{
public:
	IslandComponent(xy::MessageBus& mb, int seed);
	~IslandComponent();

    void onStart(xy::Entity& ent) override;

	Component::Type type() const {return Component::Type::Drawable;}
	void entityUpdate(xy::Entity& entity, float dt) {}

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    void updateSeaLevel(float seaLevel);

	sf::FloatRect localBounds() const override;
	sf::FloatRect globalBounds() const override;

    ///checks if a voronoi cell is land or not
    ///would prefer param to be const, but voronoi library needs changing
    bool isLand(const Cell& cell);

    ///checks if a world position is within this island
    bool isLand(const sf::Vector2f& position);


private:

    void create(Diagram* diagram);

    void getRandomSites(int count);
    int                         m_seed;
    std::vector<Point2>         m_sites;
    std::vector<sf::Vertex>     m_points;
    std::vector<sf::Vertex>     m_voronoiLines;
    std::vector<sf::Vertex>     m_triLines;
    BoundingBox                 m_bounds;
    xy::Entity*                 m_entity;

    VoronoiDiagramGenerator vGen;
    Diagram*                m_currentDiagram;

    //data for the island
    std::vector<sf::Vertex>  m_landPolys;   
    FastNoise                m_noise;
    float                    m_seaLevel;
};

