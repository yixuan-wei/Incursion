#pragma once

#include "Game/Entity.hpp"

class Bullet : public Entity
{
public:
	Bullet(Map* map, const Vec2& startPosition, EntityFaction faction, EntityType type );
	~Bullet() = default;

	virtual void Update( float deltaSeconds ) override;
	virtual void Render() const override;
	virtual void Die() override;
};
