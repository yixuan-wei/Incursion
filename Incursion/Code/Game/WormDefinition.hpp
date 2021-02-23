#pragma once

enum TileType : int;

struct WormDefinition
{
public:
	TileType wormTile;
	int numWorms;
	int wormLength;

	explicit WormDefinition( TileType tile, int numbers, int length );
};
