#pragma once

#include <xygine/components/Component.hpp>
#include <xygine/Resource.hpp>
#include <xygine/Entity.hpp>
#include "WorldController.hpp"
#include "IslandComponent.hpp"
#include <xygine/components/SfDrawableComponent.hpp>
#include <SFML/Graphics/Sprite.hpp>

class PlayerController : public xy::Component
{
public:
	PlayerController(xy::MessageBus& mb, WorldController&); //controller ref should be const but can't bc voronoi lib
	~PlayerController();
	void onStart(xy::Entity& entity) override;
	void entityUpdate(xy::Entity& entity, float dt) override;

	xy::Component::Type type() const { return xy::Component::Type::Script; }

private:
    xy::Entity*                 m_entity;
    xy::Entity*                 m_closestIsland;
    std::vector<xy::Entity*>    m_islandsInRange;
    WorldController*            m_world;
    xy::TextureResource         m_playerTextures;
    bool                        m_onLand;

    //DEBUG
    xy::SfDrawableComponent<sf::VertexArray>*       m_debugLines;
    CellType                                        m_currentCell;
    std::array<sf::Uint8,256*256*4>                 m_nearestIslandHeightMap; //1024x1024 height map for debugging
    sf::Texture                                     m_heightMapTexture;
    sf::Sprite                                      m_heightMapSprite;
};

