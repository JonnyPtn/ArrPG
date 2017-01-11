#pragma once

#include <xygine/components/Component.hpp>
#include <xygine/components/SfDrawableComponent.hpp>
#include <xygine/physics/CollisionRectangleShape.hpp>
#include <xygine/Resource.hpp>

class BoatComponent : public xy::Component, public sf::Drawable, public sf::Transformable
{
public:
    BoatComponent(xy::MessageBus& mb, xy::Entity& ent,xy::Physics::RigidBody* body,xy::TextureResource& tex);
    ~BoatComponent();

    // Inherited via Component
    virtual Type type() const override;
    virtual void entityUpdate(xy::Entity &, float) override;


    void onDelayedStart(xy::Entity& ent) override;

private:
    xy::TextureResource&		                 m_textures; 
    sf::RectangleShape                           m_drawable;
    xy::Physics::CollisionRectangleShape*        m_colShape;
    xy::Physics::RigidBody*                      m_body;
    bool                                         m_active;

    // Inherited via Drawable
    virtual void draw(sf::RenderTarget & target, sf::RenderStates states) const override;
};

