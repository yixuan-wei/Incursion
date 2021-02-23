#pragma once
#include "Engine/Math/IntVec2.hpp"

struct AABB2;
struct Rgba8;
struct Vec2;

enum TileType : int
{
	TILE_TYPE_GRASS = 0,
	TILE_TYPE_STONE,
	TILE_TYPE_MUD,
	TILE_TYPE_GROUND,
	TILE_TYPE_EXIT,
	TILE_TYPE_SAND,
	TILE_TYPE_DIRT,
	TILE_TYPE_BRICK,
	TILE_TYPE_WATER,
	TILE_TYPE_STEEL,
	TILE_TYPE_QUARTZ,
	
	NUM_TILE_TYPE
};

class Tile
{
public:
	//default tile type is grass.
	Tile(int posX, int posY);
	~Tile()=default;

	void Render()const;

	//origin is at the left bottom corner.
	AABB2 GetBounds() const;
	void GetUVCoords(Vec2& out_uvAtMins, Vec2& uvAtMaxs) const;

public:
	//every tile is 1x1 in definition
	IntVec2  m_tileCoords;
	TileType m_type;
};
