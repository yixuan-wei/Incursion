#include "Game/World.hpp"
#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Game/Tile.hpp"
#include "Game/WormDefinition.hpp"

World::World(Game* game)
	:m_game(game)
{
	Map* tempMap = nullptr;
	//map 1
	tempMap = new Map( m_game, this, IntVec2( 20, 30 ) );
	std::vector<WormDefinition> worms;
	worms.push_back( WormDefinition( TILE_TYPE_STONE, 30, 6 ) );
	worms.push_back( WormDefinition( TILE_TYPE_MUD, 20, 7 ) );
	tempMap->GenerateMap( TILE_TYPE_GRASS, TILE_TYPE_STONE, TILE_TYPE_GROUND, TILE_TYPE_GROUND, worms );
	m_maps.push_back( tempMap );
	//map 2
	tempMap = new Map( m_game, this, IntVec2( 30, 20 ) );
	worms.clear();
	worms.push_back( WormDefinition( TILE_TYPE_BRICK, 45, 5 ) );
	worms.push_back( WormDefinition( TILE_TYPE_SAND, 20, 7 ) );
	tempMap->GenerateMap( TILE_TYPE_DIRT, TILE_TYPE_BRICK, TILE_TYPE_GROUND, TILE_TYPE_GROUND, worms );
	m_maps.push_back( tempMap );
	//map 3
	tempMap = new Map( m_game, this, IntVec2( 30, 20 ) );
	worms.clear();
	worms.push_back( WormDefinition( TILE_TYPE_STEEL, 65, 5 ) );
	worms.push_back( WormDefinition( TILE_TYPE_WATER, 30, 7 ) );
	tempMap->GenerateMap( TILE_TYPE_QUARTZ, TILE_TYPE_STEEL, TILE_TYPE_GROUND, TILE_TYPE_GROUND, worms );
	m_maps.push_back( tempMap );
}

World::~World()
{
	for( int mapID = 0; mapID < (int)m_maps.size(); mapID++ )
	{
		delete m_maps[mapID];
		m_maps[mapID] = nullptr;
	}
	m_maps.clear();
	m_currentMap = nullptr;
}

void World::StartLevel()
{
	for( int mID = 0; mID < m_maps.size(); mID++ )
	{
		if( m_maps[mID] != nullptr )
			m_maps[mID]->ClearEntities();
	}
	//current
	m_currentMap = m_maps[0];
	m_currentMap->StartUp( 5, 5, 30 );//first level setup
	m_game->m_playerRespawnChances = PLAYER_RESPAWN_TIMES;
	m_currentMap->SpawnPlayer( FACTION_GOOD, Vec2( 1.5f, 1.5f ) );
}

void World::LoadNextLevel()
{
	Entity* prevPlayer = nullptr;
	prevPlayer = m_currentMap->GetPlayerAlive();
	m_currentMap->m_entityListsByType[ENTITY_TYPE_PLAYER].clear();
	for( int mapID = 0; mapID < (int)m_maps.size(); mapID++ )
	{
		if( m_maps[mapID] == m_currentMap )
		{
			if( mapID ==0 )
			{
				m_currentMap = m_maps[mapID + 1];
				m_currentMap->StartUp( 10, 10, 30 );//second level setup
				break;
			}
			else if( mapID == 1 )
			{
				m_currentMap = m_maps[mapID + 1];
				m_currentMap->StartUp( 15, 15, 60 );//third level setup
				break;
			}
			else//all map is finished, win
			{
				m_game->ProgressToState( GAME_STATE_WIN );
				return;
			}
		}
	}
	prevPlayer->m_position = Vec2( 1.5f, 1.5f );
	prevPlayer->UpdateMapPointer( m_currentMap );
	m_currentMap->AddEntityToMap( prevPlayer );
}

void World::Update( float deltaSeconds )
{
	m_currentMap->Update( deltaSeconds );
}

void World::Render() const
{
	m_currentMap->Render();
}
