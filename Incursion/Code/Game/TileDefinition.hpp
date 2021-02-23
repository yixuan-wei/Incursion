#pragma once

#include <vector>
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Game/Tile.hpp"

class TileDefinition
{
public:
	static std::vector<TileDefinition> s_definitions;
	static void InitializeDefinitions();

	TileDefinition( TileType type, Rgba8 color, bool isSolid, const SpriteDefinition& sprite, float speedFactor, bool isEnemySpawnable );

	const TileType m_type;
	const Rgba8 m_tint;
	const bool m_isSolid = false;
	const float m_speedFactor = 1.f;
	const bool m_isEnemySpawnable = false;
	//Sprite sheet UVs
	const Texture& m_texture;
	Vec2 m_uvAtMins;
	Vec2 m_uvAtMaxs;
};