#pragma once

#include "Game/Entity.hpp"

class Boulder : public Entity
{
public:
	Boulder( Map* map, const Vec2& startPos, EntityFaction faction, EntityType type );

	virtual void Render() const override;
};