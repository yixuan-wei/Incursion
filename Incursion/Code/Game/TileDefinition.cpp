#include "Game/TileDefinition.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"

std::vector<TileDefinition> TileDefinition::s_definitions;

void TileDefinition::InitializeDefinitions()
{
	s_definitions.clear();
	//load in sprite sheet
	Texture* terrainTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Terrain_8x8.png" );
	//instantiate all tiles
	SpriteSheet* terrainSheet = new SpriteSheet( *terrainTexture, IntVec2( 8, 8 ) );
	const SpriteDefinition& grass = terrainSheet->GetSpriteDefinition( 0 );  
	const SpriteDefinition& stone = terrainSheet->GetSpriteDefinition( 31 );
	const SpriteDefinition& mud = terrainSheet->GetSpriteDefinition( 21 );
	const SpriteDefinition& ground = terrainSheet->GetSpriteDefinition( 51 );
	const SpriteDefinition& exit = terrainSheet->GetSpriteDefinition( 57 );
	const SpriteDefinition& sand = terrainSheet->GetSpriteDefinition( 15 );
	const SpriteDefinition& dirt = terrainSheet->GetSpriteDefinition( 19 );
	const SpriteDefinition& brick = terrainSheet->GetSpriteDefinition( 40 );
	const SpriteDefinition& water = terrainSheet->GetSpriteDefinition( 61 );
	const SpriteDefinition& steel = terrainSheet->GetSpriteDefinition( 53 );
	const SpriteDefinition& quartz = terrainSheet->GetSpriteDefinition( 52 );
	//                                        TileType          Tint         Solid  def     Speed Enemy
	s_definitions.push_back( TileDefinition( TILE_TYPE_GRASS,  Rgba8::WHITE, false, grass,  1.f,  true ) );	
	s_definitions.push_back( TileDefinition( TILE_TYPE_STONE,  Rgba8::WHITE, true,  stone,  0.f,  false ) );
	s_definitions.push_back( TileDefinition( TILE_TYPE_MUD,    Rgba8::WHITE, false, mud,    .5f,  true ) );
	s_definitions.push_back( TileDefinition( TILE_TYPE_GROUND, Rgba8::WHITE, false, ground, 1.f,  false ) );
	s_definitions.push_back( TileDefinition( TILE_TYPE_EXIT,   Rgba8::WHITE, false, exit,   1.f,  false ) );
	s_definitions.push_back( TileDefinition( TILE_TYPE_SAND,   Rgba8::WHITE, false, sand,   .5f,  true ) );
	s_definitions.push_back( TileDefinition( TILE_TYPE_DIRT,   Rgba8::WHITE, false, dirt,   1.f,  true ) );
	s_definitions.push_back( TileDefinition( TILE_TYPE_BRICK,  Rgba8::WHITE, true,  brick,  0.f,  false ) );
	s_definitions.push_back( TileDefinition( TILE_TYPE_WATER,  Rgba8::WHITE, false, water,  .5f,  true ) );
	s_definitions.push_back( TileDefinition( TILE_TYPE_STEEL,  Rgba8::WHITE, true,  steel,  0.f,  false ) );
	s_definitions.push_back( TileDefinition( TILE_TYPE_QUARTZ, Rgba8::WHITE, false, quartz, 1.f,  true ) );

	if( s_definitions.size() != NUM_TILE_TYPE )
		FatalError( "Game/TileDefinition.cpp", "InitializeDefinitions", 24, Stringf( "Tile Definition size inconsistent with Tile type number" ) );
}

TileDefinition::TileDefinition( TileType type, Rgba8 color, bool isSolid, const SpriteDefinition& sprite, float speedFactor, bool isEnemySpawnable )
	:m_type(type)
	,m_tint(color)
	,m_isSolid(isSolid)
	, m_texture( sprite.GetTexture() )
	,m_speedFactor(speedFactor)
	,m_isEnemySpawnable(isEnemySpawnable)
{
	sprite.GetUVs( m_uvAtMins, m_uvAtMaxs );
}
