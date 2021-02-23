#pragma once

#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"

class Bomb:public Entity
{
public:
	Bomb( Map* map, const Vec2& startPos, EntityFaction faction, EntityType type );
	~Bomb() = default;

	virtual void Update( float deltaSeconds )override;
	virtual void Render()const override;
	virtual void Die() override;
};