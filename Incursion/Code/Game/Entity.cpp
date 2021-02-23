#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"
#include "Game/Pickup.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Math/MathUtils.hpp"

//////////////////////////////////////////////////////////////////////////
Entity::Entity( Map* map, const Vec2& startPosition, EntityFaction faction, EntityType type )
	:m_position(startPosition)
	, m_faction(faction)
	, m_theMap(map)
	,m_type(type)
{
}

//////////////////////////////////////////////////////////////////////////
void Entity::PickupStuff(PickupType type )
{
	if( type == PICKUP_HEALTH )
	{
		m_health += PICKUP_HEALTH_VALUE;
		if( m_health > m_healthLimit )
			m_health = m_healthLimit;
	}
	else if( type == PICKUP_FACTION_BOMB )
	{
		m_factionBombNum += 1;
	}
}

//////////////////////////////////////////////////////////////////////////
void Entity::SwitchFaction()
{
	m_faction = GetOppositeFaction();
	m_theMap->SpawnExplosion( m_position, m_cosmeticRadius, .5f * EXPLOSION_MAX_DURATION, GetFactionColor() );
}

//////////////////////////////////////////////////////////////////////////
void Entity::Update( float deltaSeconds ) 
{
	m_livingTime += deltaSeconds;

	m_orientationDegrees += m_angularVelocity * deltaSeconds;

	m_velocity += m_acceleration * deltaSeconds;
	m_velocity.ClampLength( m_speedLimit );

	m_position += m_velocity * deltaSeconds;
}

//////////////////////////////////////////////////////////////////////////
void Entity::Render() const
{
	std::vector<Vertex_PCU> drawVerts = m_verts;
	TransformVertexArray( (int)drawVerts.size(), &drawVerts[0], 1.f, m_orientationDegrees, m_position );
	g_theRenderer->DrawVertexArray( drawVerts );
}

//////////////////////////////////////////////////////////////////////////
void Entity::DebugRender() const
{
	Rgba8 cyan = Rgba8( 0, 255, 255 );	
	g_theRenderer->DrawRing2D( m_position, m_physicsRadius, LINE_THICKNESS, cyan);

	Rgba8 magenta = Rgba8( 255, 0, 255 );
	g_theRenderer->DrawRing2D( m_position, m_cosmeticRadius, LINE_THICKNESS, magenta );	

	Rgba8 yello = Rgba8( 255, 255, 0 );
	g_theRenderer->DrawLine2D( m_position, m_position + m_velocity, LINE_THICKNESS, yello);
}

//////////////////////////////////////////////////////////////////////////
void Entity::Die()
{
	m_isDead = true;
}

//////////////////////////////////////////////////////////////////////////
void Entity::TakeDamage( int damagePoints )
{
	m_health -= damagePoints;
	if( m_health <= 0 )
	{
		Die();
	}
}

//////////////////////////////////////////////////////////////////////////
void Entity::UpdateMapPointer( Map* newMap )
{
	m_theMap = newMap;
}

//////////////////////////////////////////////////////////////////////////
const EntityFaction Entity::GetOppositeFaction() const
{
	if( m_faction == FACTION_GOOD )
		return FACTION_EVIL;
	else if( m_faction == FACTION_EVIL )
		return FACTION_GOOD;
	else
		return m_faction;
}

//////////////////////////////////////////////////////////////////////////
const Rgba8 Entity::GetFactionColor() const
{
	switch( m_faction )
	{
		case FACTION_GOOD:
		{
			return Rgba8::BLUE;
		}
		case FACTION_EVIL:
		{
			return Rgba8::RED;
		}
	}
	return Rgba8::WHITE;
}

//////////////////////////////////////////////////////////////////////////
const Vec2 Entity::GetForwardVector() const
{
	return m_velocity;
}

//////////////////////////////////////////////////////////////////////////
const bool Entity::IsAlive() const
{
	if( m_isDead || m_isGarbage )
	{
		return false;
	}
	else
	{
		return true;
	}
}
