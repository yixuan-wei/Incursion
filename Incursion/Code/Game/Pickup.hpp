#pragma once

#include "Game/Entity.hpp"

enum PickupType
{
	PICKUP_HEALTH=0,
	PICKUP_FACTION_BOMB,
	NUM_PICKUP
};

class Pickup : public Entity
{
public:
	Pickup( Map* map, const Vec2& startPos, EntityFaction faction, EntityType entityType);
	~Pickup() = default;

	void Startup( PickupType pickupType );

	virtual void Render() const override;

	PickupType m_pickupType = NUM_PICKUP;
	
};