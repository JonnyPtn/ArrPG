#pragma once

#include <xygine/components/Component.hpp>
#include <xygine/Scene.hpp>
#include <xygine/Entity.hpp>
#include <VoronoiDiagramGenerator.h>
#include <SFML/Graphics/CircleShape.hpp>
#include "FastNoise.h"

//our cell types
enum class CellType
{
    OCEAN,
    LAKE,
    LAND,
    COAST
};

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

    ///tells you if a cell can reach the ocean (for lake checking)
    template<typename T>
    bool    canReachOcean(const Cell<T>& cell, std::unordered_set<const Cell<float>*>& checkedCells);

    ///checks if a world position is within this island
    bool isLand(const sf::Vector2f& position);

    ///checks if a cell is above sea level
    bool isLand(const Cell<float>* cell);


private:

    template<typename T>
    void create(Diagram<T>* diagram); //create the island based on a voronoi diagram
    float getHeight(sf::Vector2f pos); //get the height of a co-ordinate on this island
    void getRandomSites(int count); //generate some random sites for the voronoi

    //because I'm a genius these need to be called in order

    //add the ocean cells
    void addOceanAndCoastCells();

    //add the land cells
    void addLandAndLakeCells();

    int                               m_seed;
    std::vector<sf::Vector2<float>>   m_sites;
    sf::Rect<float>                   m_bounds;
    xy::Entity*                       m_entity;
    std::unordered_map<const Cell<float>*,CellType>      m_cellTypes; 

    VoronoiDiagramGenerator<float>  vGen;
    Diagram<float>*                 m_currentDiagram;

    //data for the island
    std::vector<sf::Vertex>  m_landPolys;   
    FastNoise                m_noise;
    static float             m_seaLevel;
};

