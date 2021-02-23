#include "Game/Explosion.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/RenderContext.hpp"

//////////////////////////////////////////////////////////////////////////
Explosion::Explosion( Map* map, const Vec2& position, EntityFaction faction, EntityType type )
	:Entity(map,position,faction, type)
{
	m_position = position;
	m_orientationDegrees = g_theGame->m_RNG->RollRandomFloatInRange( 0.f, 360.f );
}

//////////////////////////////////////////////////////////////////////////
void Explosion::Update( float deltaSeconds )
{
	if( m_livingTime > m_duration )
	{
		Die();
	}

	if( !IsAlive() )
		return;

	//calculate the sprite renderer
	SpriteDefinition animDef = m_anim->GetSpriteDefAtTime( m_livingTime );
	Vec2 uvAtMins, uvAtMaxs;
	animDef.GetUVs( uvAtMins, uvAtMaxs );
	m_verts.clear();
	AppendVertsForAABB2D( m_verts, AABB2( - m_cosmeticRadius, - m_cosmeticRadius,
		+ m_cosmeticRadius, + m_cosmeticRadius ), uvAtMins, uvAtMaxs, m_tint );

	Entity::Update( deltaSeconds );
}

//////////////////////////////////////////////////////////////////////////
void Explosion::Render() const
{
	Texture* animTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Explosion_5x5.png" );
	g_theRenderer->BindDiffuseTexture( animTexture );
	Entity::Render();
}

//////////////////////////////////////////////////////////////////////////
void Explosion::Startup( float radius, float durationSeconds, const Rgba8& tint )
{
	m_tint = tint;
	m_cosmeticRadius = radius;
	m_duration = durationSeconds;

	Texture* animTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Explosion_5x5.png" );
	SpriteSheet* animSheet = new SpriteSheet( *animTexture, IntVec2( 5, 5 ) );
	m_anim = new SpriteAnimDefinition( *animSheet, 0, 24, m_duration, eSpriteAnimPlaybackType::ONCE );

	AppendVertsForAABB2D( m_verts, AABB2( 0, 0, 0, 0 ) );
}

