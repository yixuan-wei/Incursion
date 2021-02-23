#include "Game/Bomb.hpp"
#include "Game/Map.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Math/AABB2.hpp"

//////////////////////////////////////////////////////////////////////////
Bomb::Bomb( Map* map, const Vec2& startPos, EntityFaction faction, EntityType type )
	:Entity(map,startPos,faction,type)
{
	m_speedLimit = BULLET_SPEED;
	m_cosmeticRadius = PICKUP_RADIUS;
	m_physicsRadius = PICKUP_RADIUS;
	m_isHitByBullets = true;

	Texture* texture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Extras_4x4.png" );
	const SpriteSheet* sheet = new SpriteSheet( *texture, IntVec2( 4, 4 ) );
	Vec2 uvAtMins, uvAtMaxs;
	sheet->GetSpriteUVs( uvAtMins, uvAtMaxs, 13 );
	Rgba8 tint = GetFactionColor();
	AppendVertsForAABB2D( m_verts, AABB2( -m_cosmeticRadius, -m_cosmeticRadius, m_cosmeticRadius, m_cosmeticRadius ),
		uvAtMins, uvAtMaxs, tint );
}

//////////////////////////////////////////////////////////////////////////
void Bomb::Update( float deltaSeconds )
{
	if( m_livingTime > BOMB_EXPLOSION_TIME&& m_theMap->IsPointInSolid( m_position ) )
	{
		Die();
	}

	if( !IsAlive() )
		return;

	Entity::Update( deltaSeconds );
}

//////////////////////////////////////////////////////////////////////////
void Bomb::Render() const
{
	Texture* texture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Extras_4x4.png" );
	g_theRenderer->BindDiffuseTexture( texture );
	Entity::Render();
}

//////////////////////////////////////////////////////////////////////////
void Bomb::Die()
{
	Entity::Die();

	m_theMap->ResolveFactionBombExlopsion( m_faction, m_position, BOMB_EXPLOSION_RADIUS );

	Rgba8 tint = GetFactionColor();
	tint.a = 100;
	m_theMap->SpawnExplosion( m_position, BOMB_EXPLOSION_RADIUS, .4f * EXPLOSION_MAX_DURATION,tint );
}
