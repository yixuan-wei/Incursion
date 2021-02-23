#pragma once

#include "Game/Entity.hpp"

struct Vec2;
class SpriteAnimDefinition;

class Explosion:public Entity
{
public:
	explicit Explosion( Map* map, const Vec2& position, EntityFaction faction, EntityType type);

	virtual void Update( float deltaSeconds ) override;
	virtual void Render() const override;

	void Startup( float radius, float durationSeconds, const Rgba8& tint = Rgba8::WHITE );

private:
	float m_duration = 0.f;
	Rgba8 m_tint = Rgba8::WHITE;
	SpriteAnimDefinition* m_anim = nullptr;
};