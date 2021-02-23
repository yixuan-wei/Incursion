#pragma once

#include "Game/Entity.hpp"

class Map;
struct Vec2;

class NpcTurret : public Entity
{
public:
	NpcTurret(Map* map, const Vec2& startPos, EntityFaction faction, EntityType type );
	~NpcTurret()=default;
	
	virtual void Update( float deltaSeconds ) override;
	virtual void Render() const override;
	virtual void TakeDamage( int damage )override;
	virtual void Die() override;

private:
	float m_shootCountdown = 0.f;
	Vec2  m_impactedPos;
	float m_lastEnemySeenDegrees = 0.f;
	bool  m_enemySeen = false;

	void ShootBullet();
};
