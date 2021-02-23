#pragma once

#include "Game/Entity.hpp"

struct Vec2;

class Player : public Entity
{
public:
	Player(Map* map, const Vec2& startPos, EntityFaction faction, EntityType type);
	~Player() = default;

	virtual void Update( float deltaSeconds ) override;
	virtual void Render()const override;
	virtual void DebugRender() const override;
	virtual void TakeDamage( int damage )override;
	virtual void Die()override;
	
	float GetGunAbsoluteDegrees() const { return m_gunRelativeOrientation + m_orientationDegrees; }

private:
	float m_thrustFraction = 0.f;
	int   m_controllerID = -1;
	float m_gunRelativeOrientation = 0.f;
	float m_vibrationCounter = 0.f;

	void UpdateFromController(float deltaSeconds);
	void ShootBullet();
	void ShootBomb();
};