#pragma once

#include <xygine/components/Component.hpp>
#include <xygine/Scene.hpp>
#include <xygine/Entity.hpp>
#include <VoronoiDiagramGenerator.h>
#include <SFML/Graphics/CircleShape.hpp>
#include "FastNoise.h"
#include <xygine/Resource.hpp>
#include <SFML/Graphics/Shader.hpp>

//our cell types
enum class CellType
{
    HIGH_ALTITUDE,
    LAND,
    COAST,
    OCEAN
};

class IslandComponent : public xy::Component, public sf::Drawable, public sf::Transformable
{
public:
	IslandComponent(xy::MessageBus& mb, int seed, float lowTide, float highTide, float highAltitude, float currentSeaLevel);
	~IslandComponent();

    void onStart(xy::Entity& ent) override;

	Component::Type type() const {return Component::Type::Drawable;}
    void entityUpdate(xy::Entity& entity, float dt);

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	sf::FloatRect localBounds() const override;
	sf::FloatRect globalBounds() const override;

    ///checks if a world position is within this island
    bool isLand(const sf::Vector2f& position);

    ///checks if a cell is above sea level
    bool isLand(const Cell* cell);

    ///get's the cell type for a given position
    CellType getCellType(const sf::Vector2<double>& position);

    float getHeight(sf::Vector2<double>& pos); //get the height of a co-ordinate on this island
    float getHeight(const Cell* cell); //get the height of a cell on this island 

    void setSleep(bool sleep);

    int getSeed() const {return m_seed;}

private:
    void create(Diagram* diagram); //create the island based on a voronoi diagram
    void getRandomSites(int count); //generate some random sites for the voronoi
    void updateVerts(); //update the visuals
    void updateSeaLevel(float seaLevel); //update this island with a new sea level
    void generateLoot();

    std::vector<sf::Vector2<double>>   m_sites; //points used for voronoi generation
    xy::Entity*                       m_entity; //our entity
    std::vector<CellType>             m_cellTypes; //the types of all cells, must be indexed in exactly the same order as the cells in the diagram
    std::vector<int>                  m_tidalCells; //All the cells which are affected by tide, sorted by altitude and referencing the index of the cell in the diagram
    int                               m_tidalCellIndex; //this is the index of the cell which is goint to be next affected by the tide

    VoronoiDiagramGenerator  vGen;
    Diagram*                 m_currentDiagram;

    //data for the island
    std::vector<sf::Vertex>  m_landPolys; //polys for land, outer vector is for each cell type;   
    FastNoise                m_noise;
    sf::Rect<float>          m_bounds;
    int                      m_seed;

    const float     m_lowTide;          //everything below this is always ocean
    const float     m_highTide;         //everything below this but above lowtide is coast
    const float     m_highAltitude;     //everything above this is snow (remaining cells are land)
    float           m_seaLevel;         //the current sea level, dynamic

    static xy::TextureResource  m_textures;
    sf::Shader                  m_shader;

    bool m_sleep; //are we asleep or not
    sf::Clock   m_shaderTime;
};

