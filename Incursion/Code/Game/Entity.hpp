#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include <vector>

struct Vec2;
struct Vertex_PCU;
class Entity;
class Map;
enum PickupType:int;

enum EntityType
{
	ENTITY_TYPE_PLAYER,
	ENTITY_TYPE_NPC_TANK, // split into good and evil two type
	ENTITY_TYPE_NPC_TURRET,
	ENTITY_TYPE_BOULDER,
	ENTITY_TYPE_GOOD_BULLET,
	ENTITY_TYPE_EVIL_BULLET,
	ENTITY_TYPE_BOMB,
	ENTITY_TYPE_PICKUP,
	ENTITY_TYPE_EXPLOSION,

	NUM_ENTITY_TYPES
};

enum EntityFaction
{
	FACTION_GOOD,
	FACTION_EVIL,
	FACTION_NEUTRAL,

	NUM_FACTIONS
};

typedef std::vector<Entity*> EntityList;

class Entity
{
	friend class Map;

public:
	Entity(Map* map, const Vec2& startPosition, EntityFaction faction, EntityType type);
	~Entity() = default;

	virtual void PickupStuff( PickupType type );
	virtual void SwitchFaction();

	virtual void Update( float deltaSeconds );
	virtual void Render() const;
	virtual void DebugRender() const;
	virtual void Die();
	virtual void TakeDamage( int );

	virtual void UpdateMapPointer( Map* newMap );

	const EntityFaction GetOppositeFaction() const;
	const Rgba8 GetFactionColor() const;
	const Vec2 GetForwardVector() const;
	const bool IsAlive() const;

public:
	Vec2  m_position;
	Vec2  m_velocity;
	Vec2  m_acceleration;
	std::vector<Vertex_PCU> m_verts;
	float m_orientationDegrees	= 0.f;
	float m_angularVelocity		= 0.f;
	float m_physicsRadius		= 0.f;
	float m_cosmeticRadius		= 0.f;
	float m_livingTime          = 0.f;
	float m_speedLimit          = 0.f;
	int   m_health				= 1;
	int   m_healthLimit         = 1;
	int   m_factionBombNum      = 0;
	bool  m_isDead				= false;
	bool  m_isGarbage			= false;
	Map*  m_theMap              = nullptr;
	EntityType m_type = NUM_ENTITY_TYPES;
	EntityFaction m_faction = NUM_FACTIONS;

protected:
	bool m_pushesEntities = false;
	bool m_isPushedByEntities = false;
	bool m_isPushedByWalls = false;
	bool m_isHitByBullets = false;
};
