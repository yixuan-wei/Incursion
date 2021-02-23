#include "Game/Tile.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"
#include "Game/TileDefinition.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

Tile::Tile( int posX, int posY )
	:m_tileCoords(posX,posY)
	,m_type(TILE_TYPE_GRASS)
{
}

void Tile::Render() const
{
	Vec2 uvAtMins, uvAtMaxs;
	GetUVCoords(uvAtMins,uvAtMaxs);
	AABB2 bounds = GetBounds();
	AppendVertsForAABB2D( g_tileVerts, bounds, uvAtMins, uvAtMaxs,Rgba8::WHITE);
}

AABB2 Tile::GetBounds() const
{
	float x = static_cast<float>(m_tileCoords.x);
	float y = static_cast<float>(m_tileCoords.y);
	return AABB2( x, y, x + 1.f, y + 1.f );
}

void Tile::GetUVCoords(Vec2& out_uvAtMins, Vec2& out_uvAtMaxs) const
{
	TileDefinition def = TileDefinition::s_definitions[m_type];
	out_uvAtMins = def.m_uvAtMins;
	out_uvAtMaxs = def.m_uvAtMaxs;
}
