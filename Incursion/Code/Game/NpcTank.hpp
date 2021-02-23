#pragma once

#include "Game/Entity.hpp"
#include "Game/Map.hpp"

struct Vec2;
struct RaycastResult;

class NpcTank : public Entity
{
public:
	NpcTank( Map* map, const Vec2& startPos, EntityFaction faction, EntityType type );
	
	virtual void Update( float deltaSeconds ) override;
	virtual void Render() const override;
	virtual void DebugRender() const override;
	virtual void TakeDamage( int damage )override;
	virtual void Die() override;

private:
	float m_resetGoalOrienCountdown = 0.f;
	float m_shootCountdown = 0.f;
	float m_goalOrientation = 0.f;
	bool  m_goalPosReached = true;
	bool  m_goalAngleReached = true;
	Vec2  m_goalPos;
	RaycastResult m_leftWhiskerResult;
	RaycastResult m_centerWhiskerResult;
	RaycastResult m_rightWhiskerResult;

	void UpdateForGoalPosNotReached();
	void UpdateForGoalPosReached(float deltaSeconds);
	void UpdateWhiskerDetection();

	void CheckToShoot(float deltaSeconds);
	void ShootBullet();
};