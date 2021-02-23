#include "Game/Bullet.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/RenderContext.hpp"

//////////////////////////////////////////////////////////////////////////
Bullet::Bullet( Map* map, const Vec2& startPosition, EntityFaction faction,EntityType type)
	:Entity(map,startPosition,faction,type)
{
	m_speedLimit = BULLET_SPEED;
	m_physicsRadius = BULLET_PHYSICS_RADIUS;
	m_cosmeticRadius = BULLET_COSMETIC_RADIUS;
	m_isHitByBullets = true;

	float halfBaseSize = m_cosmeticRadius;
	AABB2 bounds( Vec2( -halfBaseSize, -halfBaseSize ), Vec2( halfBaseSize, halfBaseSize ) );
	AppendVertsForAABB2D( m_verts, bounds );
}

//////////////////////////////////////////////////////////////////////////
void Bullet::Update( float deltaSeconds )
{
	if( m_theMap->IsPointInSolid( m_position ) )
	{
		Die();
	}

	if( !IsAlive() )
		return;

	Entity::Update( deltaSeconds );
}

//////////////////////////////////////////////////////////////////////////
void Bullet::Render() const
{
	Texture* bulletTexture = nullptr;
	if(m_faction==FACTION_EVIL )
		bulletTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/EnemyBullet.png" );
	else if(m_faction==FACTION_GOOD )
		bulletTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/FriendlyBullet.png" );
	g_theRenderer->BindDiffuseTexture( bulletTexture );

	Entity::Render();
}

//////////////////////////////////////////////////////////////////////////
void Bullet::Die()
{
	Entity::Die();

	m_theMap->SpawnExplosion( m_position, m_cosmeticRadius*3.f, .1f * EXPLOSION_MAX_DURATION );
}
