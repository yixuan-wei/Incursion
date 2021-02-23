#include "Game/Boulder.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/AABB2.hpp"

//////////////////////////////////////////////////////////////////////////
Boulder::Boulder( Map* map, const Vec2& startPos, EntityFaction faction, EntityType type )
	:Entity(map,startPos,faction,type)
{
	m_isPushedByWalls = true;
	m_pushesEntities = true;
	m_isPushedByEntities = true;
	m_physicsRadius = BOULDER_RADIUS;
	m_cosmeticRadius = BOULDER_RADIUS;

	float halfBaseSize = m_cosmeticRadius;
	AABB2 bounds( Vec2( -halfBaseSize, -halfBaseSize ), Vec2( halfBaseSize, halfBaseSize ) );
	Texture* extraTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Extras_4x4.png" );
	SpriteSheet* extraSheet = new SpriteSheet( *extraTexture, IntVec2( 4, 4 ) );
	Vec2 uvAtMins;
	Vec2 uvAtMaxs;
	extraSheet->GetSpriteUVs( uvAtMins, uvAtMaxs, 3 );
	AppendVertsForAABB2D( m_verts, bounds,uvAtMins,uvAtMaxs );
}

//////////////////////////////////////////////////////////////////////////
void Boulder::Render() const
{
	Texture* extraTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Extras_4x4.png" );
	g_theRenderer->BindDiffuseTexture( extraTexture );
	Entity::Render();
}
