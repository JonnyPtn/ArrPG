#pragma once
#include <xygine/components/Component.hpp>
#include <xygine/Resource.hpp>
#include <xygine/components/AnimatedDrawable.hpp>
#include <xygine/physics/CollisionRectangleShape.hpp>
class PirateComponent :
    public xy::Component
{
public:
    PirateComponent(xy::MessageBus& mb, xy::Entity& ent,xy::TextureResource& tex);
    ~PirateComponent();

    // Inherited via Component
    virtual Type type() const override;
    virtual void entityUpdate(xy::Entity &, float) override;

    void onStart(xy::Entity& ent) override;
    sf::FloatRect globalBounds() const override;

    void PirateComponent::destroy() override;

private:
    xy::TextureResource&		                 m_textures;
    xy::AnimatedDrawable*                        m_drawable;
    xy::Physics::CollisionRectangleShape*        m_colShape;
    bool                                         m_onBoat;
};

