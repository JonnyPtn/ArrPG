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
#include <xygine/components/SpriteBatch.hpp>

#include "LootComponent.hpp"


namespace
{
    const std::string noiseShader =
        R"(

///

//
// Description : Array and textureless GLSL 2D simplex noise function.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : stegu
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
//               https://github.com/stegu/webgl-noise
// 

vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec2 mod289(vec2 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec3 permute(vec3 x) {
  return mod289(((x*34.0)+1.0)*x);
}

float snoise(vec2 v)
  {
  const vec4 C = vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
                      0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
                     -0.577350269189626,  // -1.0 + 2.0 * C.x
                      0.024390243902439); // 1.0 / 41.0
// First corner
  vec2 i  = floor(v + dot(v, C.yy) );
  vec2 x0 = v -   i + dot(i, C.xx);

// Other corners
  vec2 i1;
  //i1.x = step( x0.y, x0.x ); // x0.x > x0.y ? 1.0 : 0.0
  //i1.y = 1.0 - i1.x;
  i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
  // x0 = x0 - 0.0 + 0.0 * C.xx ;
  // x1 = x0 - i1 + 1.0 * C.xx ;
  // x2 = x0 - 1.0 + 2.0 * C.xx ;
  vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;

// Permutations
  i = mod289(i); // Avoid truncation effects in permutation
  vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
		+ i.x + vec3(0.0, i1.x, 1.0 ));

  vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
  m = m*m ;
  m = m*m ;

// Gradients: 41 points uniformly over a line, mapped onto a diamond.
// The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)

  vec3 x = 2.0 * fract(p * C.www) - 1.0;
  vec3 h = abs(x) - 0.5;
  vec3 ox = floor(x + 0.5);
  vec3 a0 = x - ox;

// Normalise gradients implicitly by scaling m
// Approximation of: m *= inversesqrt( a0*a0 + h*h );
  m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );

// Compute final noise value at P
  vec3 g;
  g.x  = a0.x  * x0.x  + h.x  * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0 * dot(m, g);
}

uniform vec2 iResolution;
uniform vec2 offset;
uniform float zoom;

void main(void)
{
    float n = snoise((gl_FragCoord.xy + offset.xy)/10)*0.05;
    gl_FragColor.rgb = gl_Color.rgb * (1-n);
    gl_FragColor.a = gl_Color.a;
})";
}

constexpr float IslandSize(10000);

xy::TextureResource IslandComponent::m_textures;

IslandComponent::IslandComponent(xy::MessageBus& mb, int seed, float lowTide, float highTide, float highAltitude, float currentSeaLevel) :
    m_seed(seed),
    m_currentDiagram(nullptr),
    m_entity(nullptr),
    xy::Component(mb, this),
    vGen(),
    m_sites(248),
    m_noise(),
    m_landPolys(static_cast<int>(CellType::OCEAN)),
    m_lowTide(lowTide),
    m_highTide(highTide),
    m_highAltitude(highAltitude),
    m_seaLevel(currentSeaLevel),
    m_tidalCellIndex(0),
    m_sleep(false)
{
    m_bounds = {0.f, 0.f, IslandSize, IslandSize };

    //handle sea level changes
    xy::Component::MessageHandler handler;
    handler.id = Messages::SEA_LEVEL_CHANGED;
    handler.action = [this](xy::Component* c, const xy::Message& msg)
    {
        auto& data = msg.getData<float>();
        if(!m_sleep)
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

    //seed the engine first
    xy::Util::Random::rndEngine.seed(m_seed);

    getRandomSites(m_sites.size());

    m_noise = FastNoise();
    m_noise.SetNoiseType(FastNoise::NoiseType::SimplexFractal);
    m_noise.SetFrequency(0.3 / m_bounds.height);
    m_noise.SetFractalGain(5.0);
    m_noise.SetFractalType(FastNoise::FractalType::Billow);


    //seed to a random value
    m_noise.SetSeed(xy::Util::Random::value(0, m_seed));
    m_currentDiagram = vGen.compute(m_sites,static_cast<sf::Rect<double>>(m_bounds));

    //relax once
    auto newDiag = vGen.relax();
    delete m_currentDiagram;
    m_currentDiagram = newDiag;

    //twice
    /*newDiag = vGen.relax();
    delete m_currentDiagram;
    m_currentDiagram = newDiag;*/

    create(m_currentDiagram);

    //add to the quad tree
    auto qtc = xy::Component::create<xy::QuadTreeComponent>(getMessageBus(), m_bounds);
    ent.addComponent(qtc);

    //generate some booty
    generateLoot();

    //load the shader
    m_shader.loadFromMemory(noiseShader,sf::Shader::Fragment);
    m_shader.setUniform("iResolution", sf::Vector2f{ 1280,720 });
}

bool IslandComponent::isLand(const sf::Vector2f& position)
{
    //convert to island local co-ordinates
    auto localPos = m_entity->getTransform().getInverse().transformPoint(position);

    for (auto cell : m_currentDiagram->cells)
    {
        //first check the bounding box
        auto bounds = cell->getBoundingBox();
        if (bounds.contains(sf::Vector2<double>(localPos)))
        {
            //then check the point detection
            if (cell->pointIntersection(localPos.x, localPos.y) == 1)
                return isLand(cell);
       }
    }
    return false;
}

bool IslandComponent::isLand(const Cell* cell)
{
    //if it's an edge cell, deffo ocean
    if (cell->closeMe)
        return false;

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
    auto cornerRatio = 0.5;

    return (landCount / cornerCount) > cornerRatio;
}

CellType IslandComponent::getCellType(const sf::Vector2<double> & position)
{
    //check the entity is valid first
    if (m_entity)
    {
        //convert to island local co-ordinates
        auto localPos = m_entity->getTransform().getInverse().transformPoint(static_cast<sf::Vector2f>(position));

        int i(0);
        for (auto cell : m_currentDiagram->cells)
        {
            //first check the bounding box
            auto bounds = cell->getBoundingBox();
            if (bounds.contains(static_cast<sf::Vector2<double>>(localPos)))
            {
                //then check the point detection
                if (cell->pointIntersection(localPos.x, localPos.y) == 1)
                    return m_cellTypes[i];
            }
            i++; 
        }

        //not found for some reason, probably ocean
        return CellType::OCEAN;
    }
}

void IslandComponent::create(Diagram* diagram)
{
   if(m_currentDiagram && m_currentDiagram != diagram)
        delete m_currentDiagram;
    m_currentDiagram = diagram;

    //clear all current cells
    m_cellTypes.clear();
    m_tidalCells.clear();

    //go through all cells and assign type based on height
    int i(0);
    for (auto cell : m_currentDiagram->cells)
    {
        //force edge cells to be ocean though
        if (cell->closeMe)
        {
            m_cellTypes.push_back(CellType::OCEAN);
            continue;
        }

        auto height = getHeight(cell);

        CellType type;
        if (height < m_seaLevel)
        {
            type = CellType::OCEAN;
        }
        else if (height < m_highTide)
        {
            type = CellType::COAST;
        }
        else if (height < m_highAltitude)
        {
            type = CellType::LAND;
        }
        else
        {
            type = CellType::HIGH_ALTITUDE;
        }
        m_cellTypes.push_back(type);

        //add to the tidal cell vector if needed
        if (height > m_lowTide && height < m_highTide)
        {
            m_tidalCells.push_back(i);
        }
        i++;
    } 
    
    //sort all the tidal cells
    std::sort(m_tidalCells.begin(), m_tidalCells.end(), [this](const int a, const int b) {return getHeight(m_currentDiagram->cells[a]) < getHeight(m_currentDiagram->cells[b]); });
    updateVerts();
}

float IslandComponent::getHeight(sf::Vector2<double>& pos)
{
    sf::Vector2<double> centre;
    centre.x = localBounds().width / 2;
    centre.y = localBounds().height / 2;

    //get the distance between the centre and the position
    auto d = pos - centre;
    auto dv = xy::Util::Vector::length(sf::Vector2f(d.x, d.y));

    //make it a factor of the max distance
    auto df = 1 - (dv / (m_bounds.width));

    //if the noise value factored by the distance is over sea level, it's land
    auto n = 1 - ((m_noise.GetSimplexFractal(pos.x, pos.y) + 1) / 2);

    return df * n;
}

float IslandComponent::getHeight(const Cell* cell)
{
    float totalHeight(0.f);
    //average the corner heights for the cell height
    for (auto e : cell->halfEdges)
    {
        totalHeight += getHeight(*e->startPoint());
    }
    return totalHeight / cell->halfEdges.size();
}

void IslandComponent::setSleep(bool sleep)
{
    m_sleep = sleep;
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
    std::sort(m_sites.begin(), m_sites.end(), [](const sf::Vector2<double>& s1, const sf::Vector2<double>& s2)
    {
        if (s1.y < s2.y)
            return true;
        if (s1.y == s2.y && s1.x < s2.x)
            return true;
        return false;
    });
}

void IslandComponent::updateVerts()
{
    //create land polys
    int vertCount(0), i(0);
    m_landPolys.clear();
    for (auto& cell : m_currentDiagram->cells)
    {
        //assign the cell a type based on its height
        //assign it a cell
        //adjust the colour based on type, ignoring ocean tiles
        sf::Color cellColour;
        auto cellType = m_cellTypes[i++];
       /* if (cellType != m_cellTypes.end())
        {*/
            switch (cellType)
            {
            case CellType::COAST:
                cellColour = { 239, 221, 111 }; //sandy colour
                break;

            case CellType::LAND:
                cellColour = { 0,100,0 };
                break;

            case CellType::HIGH_ALTITUDE:
                cellColour = sf::Color::White; //snow
                break;
            }
            //add polys for everything except ocean
            if (cellType != CellType::OCEAN)
            {
                //go through all the half edges
                for (auto h : cell->halfEdges)
                {
                   // if (m_landPolys.size() <= vertCount)
                   // {
                        //add a tri using start, end and site point
                        m_landPolys.push_back({ sf::Vector2f(h->startPoint()->x,h->startPoint()->y),cellColour });
                        m_landPolys.push_back({ sf::Vector2f(h->endPoint()->x, h->endPoint()->y),cellColour });
                        m_landPolys.push_back({ sf::Vector2f(cell->site.p.x, cell->site.p.y),cellColour });
                   // }
                  /*  else
                    {
                        m_landPolys[vertCount] = {sf::Vector2f(h->startPoint()->x, h->startPoint()->y), cellColour};
                        m_landPolys[vertCount+1] = { sf::Vector2f(h->startPoint()->x, h->startPoint()->y), cellColour };
                        m_landPolys[vertCount+2] = { sf::Vector2f(h->startPoint()->x, h->startPoint()->y), cellColour };
                    }*/
                    vertCount += 3;
                }
            }
        //}
    }
 }
;

void IslandComponent::entityUpdate(xy::Entity & entity, float dt)
{
    //update shader params
    sf::Vector2f pos = entity.getScene()->getView().getCenter();
    pos.y = -pos.y;
    m_shader.setUniform("offset", pos);
    m_shader.setUniform("iResolution", entity.getScene()->getView().getSize());
}

void IslandComponent::draw(sf::RenderTarget & target, sf::RenderStates states) const
{
    if (!m_sleep)
    {
        states.transform *= getTransform();
        states.shader = &m_shader;
        target.draw(m_landPolys.data(), m_landPolys.size(), sf::PrimitiveType::Triangles, states);

        //using the palm tree texture
        states.texture = &m_textures.get("PalmTree.png");

        //debug
        //target.draw(*m_currentDiagram, states);
    }
}

void IslandComponent::updateSeaLevel(float seaLevel)
{
    //check the next closest tidal cell and adjust if necessary
    if (seaLevel > m_seaLevel)
    {
        //tide coming in

        while (getHeight(m_currentDiagram->cells[m_tidalCells[m_tidalCellIndex]]) < m_seaLevel)
        {
            m_cellTypes[m_tidalCells[m_tidalCellIndex]] = CellType::OCEAN;
            if (m_tidalCellIndex < m_tidalCells.size() - 1)
                m_tidalCellIndex++;
            else
                break;
        }
    }
    else
    {
        //tide going out
        while (getHeight(m_currentDiagram->cells[m_tidalCells[m_tidalCellIndex]]) > m_seaLevel)
        {
            m_cellTypes[m_tidalCells[m_tidalCellIndex]] = CellType::COAST;
            if (m_tidalCellIndex > 0)
                m_tidalCellIndex--;
            else
                break;
        }
    }
    m_seaLevel = seaLevel;
    updateVerts();
}

void IslandComponent::generateLoot()
{
    //get some random positions and add loot there
    //let's say... 100 wood
    const int lootCount = 100;
    for (int i = 0; i < lootCount; i++)
    {
        //create the entity
        auto lootEntity = xy::Entity::create(getMessageBus());

        //find a position
        sf::Vector2f randomPos;
        do
        {
            randomPos.x = xy::Util::Random::value(m_bounds.left, m_bounds.left + m_bounds.width);
            randomPos.y = xy::Util::Random::value(m_bounds.top, m_bounds.top + m_bounds.height);
        } while (!isLand(randomPos));
        lootEntity->setPosition(randomPos);

        //add the loot component, contains our loot (wood, would you beleive it!)
        auto lootComp = xy::Component::create<LootComponent>(getMessageBus(), std::make_unique<Wood>(), m_textures);
        lootEntity->addComponent(lootComp);

        //add it to the quad tree too
        auto qtc = xy::Component::create<xy::QuadTreeComponent>(getMessageBus(),lootEntity->globalBounds());
        lootEntity->addComponent(qtc);

        //add to the scene
        m_entity->addChild(lootEntity);
    }

    //and 100 rope
    const int ropeCount = 100;
    for (int i = 0; i < ropeCount; i++)
    {
        //create the entity
        auto lootEntity = xy::Entity::create(getMessageBus());

        //find a position
        sf::Vector2f randomPos;
        do
        {
            randomPos.x = xy::Util::Random::value(m_bounds.left, m_bounds.left + m_bounds.width);
            randomPos.y = xy::Util::Random::value(m_bounds.top, m_bounds.top + m_bounds.height);
        } while (!isLand(randomPos));
        lootEntity->setPosition(randomPos);

        //add the loot component, contains our loot (wood, would you beleive it!)
        auto lootComp = xy::Component::create<LootComponent>(getMessageBus(), std::make_unique<Rope>(), m_textures);

        //add it to the quad tree too
        auto qtc = xy::Component::create<xy::QuadTreeComponent>(getMessageBus(), lootComp->globalBounds());
        lootEntity->addComponent(qtc);

        //add to the scene
        lootEntity->addComponent(lootComp);
        m_entity->addChild(lootEntity);
    }
}

sf::FloatRect IslandComponent::localBounds() const
{
    return sf::FloatRect(m_bounds);
}

sf::FloatRect IslandComponent::globalBounds() const
{
    return getTransform().transformRect(sf::FloatRect(m_bounds));
}
