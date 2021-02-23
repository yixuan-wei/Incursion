#include "Game/Pickup.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Math/AABB2.hpp"

//////////////////////////////////////////////////////////////////////////
Pickup::Pickup( Map* map, const Vec2& startPos, EntityFaction faction, EntityType entityType)
	:Entity(map,startPos,faction,entityType)
{
	m_isPushedByWalls = true;
	m_isHitByBullets = true;
	m_physicsRadius = PICKUP_RADIUS;
	m_cosmeticRadius = PICKUP_RADIUS;
}

//////////////////////////////////////////////////////////////////////////
void Pickup::Startup( PickupType pickupType )
{
	m_pickupType = pickupType;

	Texture* texture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Extras_4x4.png" );
	const SpriteSheet* sheet = new SpriteSheet( *texture, IntVec2( 4, 4 ) );
	Vec2 uvAtMins, uvAtMaxs;
	if( m_pickupType == PICKUP_HEALTH )
	{
		sheet->GetSpriteUVs( uvAtMins, uvAtMaxs, 11 );
	}
	else if( m_pickupType == PICKUP_FACTION_BOMB )
	{
		sheet->GetSpriteUVs( uvAtMins, uvAtMaxs, 13 );
	}

	Rgba8 tint = GetFactionColor();
	AppendVertsForAABB2D( m_verts, AABB2( -m_cosmeticRadius, -m_cosmeticRadius, m_cosmeticRadius, m_cosmeticRadius ),
		uvAtMins, uvAtMaxs, tint );
}

//////////////////////////////////////////////////////////////////////////
void Pickup::Render() const
{
	Texture* pickupTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Extras_4x4.png" );
	g_theRenderer->BindDiffuseTexture( pickupTexture );
	Entity::Render();
}
