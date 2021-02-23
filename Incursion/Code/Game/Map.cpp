#include "Game/Map.hpp"
#include "Game/Tile.hpp"
#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/Player.hpp"
#include "Game/NpcTurret.hpp"
#include "Game/NpcTank.hpp"
#include "Game/Boulder.hpp"
#include "Game/Bullet.hpp"
#include "Game/Bomb.hpp"
#include "Game/Explosion.hpp"
#include "Game/Pickup.hpp"
#include "Game/World.hpp"
#include "Game/TileDefinition.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"

std::vector<Vertex_PCU> g_tileVerts;

Map::Map( Game* game, World* world, const IntVec2& tileDimension )
	:m_world(world)
	,m_game(game)
	,m_size(tileDimension)
{
}

Map::~Map()
{
	for( int entityTypeID = 0; entityTypeID < (int)NUM_ENTITY_TYPES; entityTypeID++ )
	{
		EntityList& entityList = m_entityListsByType[entityTypeID];
		if(entityTypeID==ENTITY_TYPE_PLAYER )
			continue;
		for( int entityID = 0; entityID < (int)entityList.size(); entityID++ )
		{
			delete entityList[entityID];
		}
		entityList.clear();
	}
	
	m_tiles.clear();
}

void Map::StartUp(int turretNum, int tankNum, int boulderNum)
{
	for( int turretID = 0; turretID < turretNum; turretID++ )
	{
		SpawnNPC( ENTITY_TYPE_NPC_TURRET, FACTION_EVIL );
	}
	ResolveTurretsOverlap();
	for( int tankID = 0; tankID < tankNum; tankID++ )
	{
		SpawnNPC( ENTITY_TYPE_NPC_TANK, FACTION_EVIL );
	}
	for( int boulderID = 0; boulderID < boulderNum; boulderID++ )
	{
		SpawnNPC( ENTITY_TYPE_BOULDER, FACTION_NEUTRAL );
	}
}

Entity* Map::SpawnNewEntity( EntityType type, EntityFaction faction, const Vec2& spawnPosition )
{
	Entity* newEntity = nullptr;
	switch( type )
	{
		case ENTITY_TYPE_PLAYER:      newEntity = new Player(this, spawnPosition, faction, type );  break;
		case ENTITY_TYPE_GOOD_BULLET: newEntity = new Bullet( this, spawnPosition, faction, type ); break;
		case ENTITY_TYPE_EVIL_BULLET: newEntity = new Bullet( this, spawnPosition, faction, type ); break;
		case ENTITY_TYPE_NPC_TURRET:  newEntity = new NpcTurret(this, spawnPosition, faction, type); break;
		case ENTITY_TYPE_NPC_TANK:    newEntity = new NpcTank(this, spawnPosition, faction, type ); break;
		case ENTITY_TYPE_BOULDER:     newEntity = new Boulder( this, spawnPosition, faction, type ); break;
		case ENTITY_TYPE_PICKUP:      newEntity = new Pickup( this, spawnPosition, faction, type ); break;
		case ENTITY_TYPE_BOMB:        newEntity = new Bomb( this, spawnPosition, faction, type );   break;
		case ENTITY_TYPE_EXPLOSION:   newEntity = new Explosion( this, spawnPosition, faction, type ); break;
	}
	if(newEntity!=nullptr )
		AddEntityToMap( newEntity );
	return newEntity;
}

Entity* Map::SpawnBullet( EntityType type, EntityFaction faction, const Vec2& spawnPos, float orientation )
{
	Entity* newBullet = SpawnNewEntity( type, faction, spawnPos );
	newBullet->m_orientationDegrees = orientation;
	newBullet->m_velocity = BULLET_SPEED * Vec2::MakeFromPolarDegrees( orientation );
	return newBullet;
}

Entity* Map::SpawnBomb( EntityFaction faction, const Vec2& spawnPos, float orientation )
{
	Entity* newBomb = SpawnNewEntity( ENTITY_TYPE_BOMB, faction, spawnPos );
	newBomb->m_orientationDegrees = orientation;
	newBomb->m_velocity = BULLET_SPEED * Vec2::MakeFromPolarDegrees( orientation );
	return newBomb;
}

Entity* Map::SpawnPlayer( EntityFaction faction, const Vec2& preferSpawnPosition )
{
	m_game->m_playerRespawnChances--;
	if( m_game->m_playerRespawnChances < 0 )
	{
		m_game->ProgressToState( GAME_STATE_LOSE );
		return nullptr;
	}
	//guarantee that only one player alive
	//spawn new player at old player's position
	Vec2 trueSpawnPos = preferSpawnPosition;
	EntityList& playerList = m_entityListsByType[ENTITY_TYPE_PLAYER];
	for( int pID = 0; pID < (int)playerList.size(); pID++ )
	{
		if( playerList[pID] != nullptr )
		{
			trueSpawnPos = playerList[pID]->m_position;
			delete playerList[pID];
			playerList[pID] = nullptr;
		}
	}
	Entity* player = SpawnNewEntity( ENTITY_TYPE_PLAYER, faction, trueSpawnPos );
	
	return player;
}

Entity* Map::SpawnExplosion( const Vec2& spawnPosition, float radius, float duration, const Rgba8& tint/*= Rgba8::WHITE*/ )
{
	Explosion* newExplosion = (Explosion*)SpawnNewEntity( ENTITY_TYPE_EXPLOSION, FACTION_NEUTRAL, spawnPosition );
	newExplosion->Startup( radius, duration, tint );
	return (Entity*)newExplosion;
}

Entity* Map::SpawnPickup( EntityFaction faction, const Vec2& spawnPosition )
{
	Pickup* newPickup = (Pickup*) SpawnNewEntity( ENTITY_TYPE_PICKUP, faction, spawnPosition );
	PickupType type = PICKUP_HEALTH;
	float pickupFactor = g_theGame->m_RNG->RollRandomFloatZeroToOneInclusive();
	if( pickupFactor > .8f )//switch to spawn allay pickup
		type = PICKUP_FACTION_BOMB;
	newPickup->Startup( type );
	return (Entity*)newPickup;
}

void Map::AddEntityToMap( Entity* entity )
{
	EntityList& myList = m_entityListsByType[entity->m_type];
	AddEntityToList( entity, myList );
}

void Map::AddEntityToList( Entity* entity, EntityList& entityList )
{
	for( int eID = 0; eID < (int)entityList.size(); eID++ )
	{
		if( entityList[eID] == nullptr )
		{
			entityList[eID] = entity;
			return;
		}
	}
	entityList.push_back( entity );
}

void Map::ResolveFactionBombExlopsion( EntityFaction faction, const Vec2& position, float radius )
{
	ResolveFactionBombForEntityType( ENTITY_TYPE_NPC_TANK, faction, position, radius );
	ResolveFactionBombForEntityType( ENTITY_TYPE_NPC_TURRET, faction, position, radius );
}

int Map::GetTileIndexForTileCoords( const IntVec2& tileCoords ) const
{
	int index = m_size.x * tileCoords.y + tileCoords.x;
	int totalNums = m_size.x * m_size.y - 1;
	if( index > totalNums )
		index = totalNums;
	return index;
}

int Map::GetTileIndexForPosition( const Vec2& position ) const
{
	IntVec2 tileCoords = GetTileCoordsForPosition( position );
	return GetTileIndexForTileCoords( tileCoords );
}

IntVec2 Map::GetTileCoordsForTileIndex( int tileIndex ) const
{
	int y = tileIndex / m_size.x;
	int x = tileIndex - y * m_size.x;
	return IntVec2( x, y );
}

IntVec2 Map::GetTileCoordsForPosition( const Vec2& position ) const
{
	int xCoord = RoundDownToInt( position.x );
	int yCoord = RoundDownToInt( position.y );
	return IntVec2( xCoord, yCoord );
}

Entity* Map::GetPlayerAlive() const
{
	EntityList playerList = m_entityListsByType[ENTITY_TYPE_PLAYER];
	if( playerList.size() < 1 )
		return nullptr;

	Entity* player = playerList[0];
	if( player!=nullptr && player->IsAlive() )
		return player;
	else return nullptr;
}

bool Map::IsPointInSolid( const Vec2& point ) const
{
	int index = GetTileIndexForPosition( point );
	return TileDefinition::s_definitions[m_tiles[index].m_type].m_isSolid;
}

bool Map::IsPointInTileType( const Vec2& point, TileType type ) const
{
	int index = GetTileIndexForPosition( point );
	if( m_tiles[index].m_type == type )
		return true;
	else return false;
}

bool Map::IsTileSolid( const IntVec2& tileCoords ) const
{
	int index = GetTileIndexForTileCoords( tileCoords );
	TileDefinition& tileDefinition = TileDefinition::s_definitions[m_tiles[index].m_type];
	return tileDefinition.m_isSolid;
}

bool Map::IsTileInEdge( const IntVec2& tileCoords ) const
{
	if( tileCoords.x == 0 || tileCoords.x == m_size.x - 1 || tileCoords.y == 0 || tileCoords.y == m_size.y - 1 )
		return true;
	return false;
}

RaycastResult Map::Raycast( const Vec2& startPosition, const Vec2& forwardDir, float maxDist ) const
{
	RaycastResult result;
	//detect solid every 1/Raycast-sample-rate world unit
	float unitStep = 1.0f / (float)RAYCAST_SAMPLE_RATE;
	float detectedLength = 0.f;
	while( detectedLength <= maxDist )
	{
		Vec2 lastDetectedPos = startPosition + detectedLength*forwardDir;
		Vec2 thisDetectPos = lastDetectedPos + forwardDir * unitStep;
		int thisTileIndex = GetTileIndexForPosition( thisDetectPos );
		TileDefinition& tileDefinition = TileDefinition::s_definitions[m_tiles[thisTileIndex].m_type];
		if( tileDefinition.m_isSolid )
		{
			AABB2 thisTileBound = m_tiles[thisTileIndex].GetBounds();
			Vec2 pointOnSolid = GetNearestPointOnAABB2D( lastDetectedPos, thisTileBound );
			result.m_impactDist = (pointOnSolid - startPosition).GetLength();
			if( result.m_impactDist > maxDist )
				break;
			result.m_impacted = true;
			result.m_impactPos = pointOnSolid;
			result.m_impactTileType = tileDefinition.m_type;			
			if( pointOnSolid.x == thisTileBound.mins.x )
			{
				result.m_impactNormal = Vec2( -1.f, 0.f );
			}
			else if( pointOnSolid.x == thisTileBound.maxs.x )
			{
				result.m_impactNormal = Vec2( 1.f, 0.f );
			}
			else if( pointOnSolid.y == thisTileBound.mins.y )
			{
				result.m_impactNormal = Vec2( 0.f, -1.f );
			}
			else result.m_impactNormal = Vec2( 0.f, 1.f );
			return result;
		}
		detectedLength += unitStep;
	}
	result.m_impacted = false;
	result.m_impactPos = startPosition + maxDist * forwardDir;
	result.m_impactDist = maxDist;
	return result;
}

Entity* Map::RaycastForEnemyFaction( EntityFaction faction, const Vec2& startPoint, float maxDist ) const
{
	Entity* result = nullptr;
	//ray cast for player
	result = RaycastForEnemyType( faction, ENTITY_TYPE_PLAYER, startPoint, maxDist );
	if( result!=nullptr )		return result;
	//ray cast for turret
	result = RaycastForEnemyType( faction, ENTITY_TYPE_NPC_TURRET, startPoint, maxDist );
	if( result != nullptr )		return result;
	//ray cast for tank
	result = RaycastForEnemyType( faction, ENTITY_TYPE_NPC_TANK, startPoint, maxDist );
	if( result != nullptr )		return result;
	//ray cast for pickups
	result = RaycastForEnemyType( faction, ENTITY_TYPE_PICKUP, startPoint, maxDist );
	if( result != nullptr )		return result;
	//return false
	return nullptr;
}

Entity* Map::RaycastForEnemyType( EntityFaction faction, EntityType type, const Vec2& startPoint, float maxDist ) const
{
	const EntityList& entityList = m_entityListsByType[type];
	for( int turID = 0; turID < (int)entityList.size(); turID++ )
	{
		Entity* result = RaycastForEnemyEntity( faction, entityList[turID], startPoint, maxDist );
		if( result!=nullptr )		return result;
	}
	return nullptr;
}

Entity* Map::RaycastForEnemyEntity( EntityFaction faction, Entity* enemy, const Vec2& startpoint, float maxDist ) const
{
	RaycastResult result;
	if( enemy == nullptr || !enemy->IsAlive()||enemy->m_faction == faction )
	{
		result.m_impacted = false;
		result.m_impactDist = maxDist;
		return nullptr;
	}
	Vec2 thisToEnemy = enemy->m_position - startpoint;
	result = Raycast( startpoint, thisToEnemy.GetNormalized(), maxDist );
	if( thisToEnemy.GetLength() < result.m_impactDist )
	{
		return enemy;
	}
	else return nullptr;
}

bool Map::HasLineOfSight( const Vec2& startPoint, const Vec2& endPoint, float maxDist ) const
{
	Vec2 forward = endPoint - startPoint;
	RaycastResult raycastResult = Raycast( startPoint, forward.GetNormalized(), maxDist );
	if( forward.GetLength()<raycastResult.m_impactDist )
		return true;
	else return false;
}

void Map::GenerateMap( TileType defaultTile, TileType edgeTile, TileType startTile, TileType endTile, std::vector<WormDefinition>& wormDefs )
{
	InitTiles( defaultTile, edgeTile, startTile, endTile, wormDefs );
	while( !IsMapWalkable() )
	{
		InitTiles( defaultTile, edgeTile, startTile, endTile, wormDefs );
	}
}

void Map::InitTiles( TileType defaultTile, TileType edgeTile, TileType startTile, TileType endTile, std::vector<WormDefinition>& wormDefs )
{
	//init all to default
	int totalSize = m_size.x * m_size.y;
	for( int tileID = 0; tileID < totalSize; tileID++ )
	{
		IntVec2 tileCoords = GetTileCoordsForTileIndex( tileID );
		Tile newTile( tileCoords.x, tileCoords.y );
		newTile.m_type = defaultTile;
		m_tiles.push_back( newTile );
	}
	//set outer frame to stone
	for( int tileXPos = 0; tileXPos < m_size.x; tileXPos++ )
	{
		m_tiles[tileXPos].m_type = edgeTile;
		int lastLinePos = GetTileIndexForTileCoords( IntVec2( tileXPos, m_size.y - 1 ) );
		m_tiles[lastLinePos].m_type = edgeTile;
	}
	for( int tileYPos = 0; tileYPos < m_size.y; tileYPos++ )
	{
		int startPos = GetTileIndexForTileCoords( IntVec2( 0, tileYPos ) );
		m_tiles[startPos].m_type = edgeTile;
		int endPos = GetTileIndexForTileCoords( IntVec2( m_size.x - 1, tileYPos ) );
		m_tiles[endPos].m_type = edgeTile;
	}
	//set worm tiles in map
	for( int wormDefID = 0; wormDefID < (int)wormDefs.size(); wormDefID++ )
	{
		InitWormsForDefinition( wormDefs[wormDefID] );
	}
	//set birth and end point to grass
	//Remember to leave the outer frame
	int vacantSize = 5+1;
	for( int xID = 1; xID < vacantSize; xID++ )
	{
		for( int yID = 1; yID < vacantSize; yID++ )
		{
			int birthPlaceID = GetTileIndexForTileCoords( IntVec2( xID, yID ) );
			int endPlaceID = totalSize - 1 - birthPlaceID;
			if( (xID == 4 && (yID > 1 && yID < 5)) || (yID == 4 && (xID > 1 && xID < 5)) )
			{
				m_tiles[birthPlaceID].m_type = edgeTile;
				m_tiles[endPlaceID].m_type = edgeTile;
			}
			else
			{
				m_tiles[birthPlaceID].m_type = startTile;
				m_tiles[endPlaceID].m_type = endTile;
			}
		}
	}
}

void Map::InitWormsForDefinition( WormDefinition wormDef )
{
	for( int wormID = 0; wormID < wormDef.numWorms; wormID++ )
	{
		//choose random start location
		int xStart = g_theGame->m_RNG->RollRandomIntInRange( 1, m_size.x - 2 );
		int yStart = g_theGame->m_RNG->RollRandomIntInRange( 1, m_size.y - 2 );
		IntVec2 currentTilePos( xStart, yStart );
		for( int lengthID = 0; lengthID < wormDef.wormLength; lengthID++ )
		{
			m_tiles[GetTileIndexForTileCoords( currentTilePos )].m_type = wormDef.wormTile;
			IntVec2 newTilePos = GetRandomAdjacentTileCoords( currentTilePos );
			while(lengthID<wormDef.wormLength && IsTileInEdge(newTilePos) )
			{
				lengthID++;
				newTilePos = GetRandomAdjacentTileCoords( currentTilePos );
			}
			currentTilePos = newTilePos;
		}
	}
}

IntVec2 Map::GetRandomAdjacentTileCoords( const IntVec2& tileCoords )
{
	float factor = g_theGame->m_RNG->RollRandomFloatZeroToOneInclusive();
	if( factor < .25f )//left
		return IntVec2( tileCoords.x - 1, tileCoords.y );
	else if( factor < .5f )//up
		return IntVec2( tileCoords.x, tileCoords.y + 1 );
	else if( factor < .75f )//right
		return IntVec2( tileCoords.x + 1, tileCoords.y );
	else //down
		return IntVec2( tileCoords.x, tileCoords.y - 1 );
}

bool Map::IsMapWalkable()
{
	//flood-fill to walk the map
	int mapSize = m_size.x * m_size.y;
	bool* isReachable = new bool[mapSize];
	bool* isProcessed = new bool[mapSize];
	//init solid tile to not reachable and is processed
	for( int tileIndex = 0; tileIndex < mapSize; tileIndex++ )
	{
		isReachable[tileIndex] = false;
		if( TileDefinition::s_definitions[m_tiles[tileIndex].m_type].m_isSolid )
		{
			isProcessed[tileIndex] = true;
		}
		else isProcessed[tileIndex] = false;
	}
	//init start point reachable 
	isReachable[m_size.x + 1] = true;
	//do flood fill until no new state can be updated
	int adjacent[4] = { -1,1,m_size.x,-m_size.x };
	bool updated = false;
	do
	{
		updated = false;
		for( int tileID = m_size.x + 1; tileID < mapSize - m_size.x - 1; tileID++ )
		{
			if( !isProcessed[tileID] && isReachable[tileID] )
			{
				updated = true;
				for( int adjID = 0; adjID < 4; adjID++ )
				{
					int adjacentTileID = tileID + adjacent[adjID];
					if( !isProcessed[adjacentTileID] ) 
						isReachable[adjacentTileID] = true;
				}
				isProcessed[tileID] = true;
			}
		}
	} while( updated );
	//detect if reachable to exit
	if( !isReachable[mapSize - m_size.x - 2] )//exit not reachable
	{
		m_tiles.clear();
		return false;
	}
	//exit reachable, then fill out not reachable area.
	do
	{
		updated = false;
		for( int tileID = m_size.x + 1; tileID < mapSize - m_size.x - 1; tileID++ )
		{
			if( !isProcessed[tileID] )
			{
				for( int adjID = 0; adjID < 4; adjID++ )
				{
					int adjacentTileID = tileID + adjacent[adjID];
					if( isProcessed[adjacentTileID] )
					{
						updated = true;
						isProcessed[tileID] = true;
						isReachable[tileID] = isReachable[adjacentTileID];
						m_tiles[tileID].m_type = m_tiles[adjacentTileID].m_type;
						break;
					}
				}
			}
		}
	} while( updated );
	return true;
}

Vec2 Map::GetEnemySpawnPoint() const
{
	Vec2 spawnPos;
	spawnPos.x = g_theGame->m_RNG->RollRandomFloatInRange( 0.f, (float)m_size.x );
	spawnPos.y = g_theGame->m_RNG->RollRandomFloatInRange( 0.f, (float)m_size.y );
	while( !TileDefinition::s_definitions[m_tiles[GetTileIndexForPosition( spawnPos )].m_type].m_isEnemySpawnable )
	{
		spawnPos.x = g_theGame->m_RNG->RollRandomFloatInRange( 0.f, (float)m_size.x );
		spawnPos.y = g_theGame->m_RNG->RollRandomFloatInRange( 0.f, (float)m_size.y );
	}
	return spawnPos;
}

float Map::GetTileSpeedFactorForPoint( const Vec2& point ) const
{
	TileType tileType = m_tiles[GetTileIndexForPosition( point )].m_type;
	return TileDefinition::s_definitions[tileType].m_speedFactor;
}

Entity* Map::SpawnNPC(EntityType type, EntityFaction faction)
{
	Vec2 spawnPos = GetEnemySpawnPoint();
	Entity* npcEntity = SpawnNewEntity( type, faction, spawnPos );
	ResolveEntityTileCollision( npcEntity );
	npcEntity->m_orientationDegrees = g_theGame->m_RNG->RollRandomFloatInRange( 0.f, 360.f );
	return npcEntity;
}


bool Map::IsLevelCompleted() const
{
	Entity* player = GetPlayerAlive();
	if( player == nullptr ) return false;
	EntityFaction myFaction = player->m_faction;
	if( IsEnemyFactionExist( myFaction ) )
		return true;
		
	else return false;
}

bool Map::IsEnemyFactionExist(EntityFaction faction) const
{
	for( int typeID = 0; typeID < (int)ENTITY_TYPE_BOULDER; typeID++ )
	{
		const EntityList& entityList = m_entityListsByType[typeID];
		for( int eID = 0; eID < (int)entityList.size(); eID++ )
		{
			const Entity* entity = entityList[eID];
			if(entity==nullptr||!entity->IsAlive() )
				continue;
			if( entity->m_faction != faction )
				return false;
		}
	}
	return true;
}

void Map::Update( float deltaSeconds )
{
	CleanDeadTrashEntities();
	if( IsLevelCompleted() )
	{
		m_world->LoadNextLevel();
		return;
	}
	UpdateEntities( deltaSeconds );	
	DetectCollisionForBombs();
	DetectCollisionForPickups();
	DetectCollisionForEntities();
	DetectCollisionForTilesAndEntities();
	
}

void Map::UpdateEntities( float deltaSeconds )
{
	for( int entityTypeID = 0; entityTypeID < (int)NUM_ENTITY_TYPES; entityTypeID++ )
	{
		EntityList& entityList = m_entityListsByType[entityTypeID];
		for( int entityID = 0; entityID < (int)entityList.size(); entityID++ )
		{
			Entity* entity = entityList[entityID];
			if( entity != nullptr && entity->IsAlive() )
			{
				if( entityTypeID == ENTITY_TYPE_PLAYER || entityTypeID == ENTITY_TYPE_NPC_TANK )
				{
					float speedFactor = GetTileSpeedFactorForPoint( entity->m_position );
					entity->Update( deltaSeconds * speedFactor );
				}
				else entity->Update( deltaSeconds );
			}
		}
	}
}

void Map::ClearEntities()
{
	for( int listID = 0; listID < (int)NUM_ENTITY_TYPES; listID++ )
	{
		EntityList& entityList = m_entityListsByType[listID];
		for( int eID = 0; eID < (int)entityList.size(); eID++ )
		{
			if(entityList[eID] != nullptr)
				delete entityList[eID];
			entityList[eID] = nullptr;
		}
	}
}

void Map::CleanDeadTrashEntities()
{
	for( int entityTypeID = 0; entityTypeID < (int)NUM_ENTITY_TYPES; entityTypeID++ )
	{
		if(entityTypeID==ENTITY_TYPE_PLAYER )
			continue;
		EntityList& entityList = m_entityListsByType[entityTypeID];
		for( int entityID = 0; entityID < (int)entityList.size(); entityID++ )
		{
			Entity* entity = entityList[entityID];
			if( entity!=nullptr && !entity->IsAlive() )
			{
				delete entity;
				entityList[entityID] = nullptr;
			}
		}
	}
}

void Map::DetectCollisionForTilesAndEntities()
{	
	//detect collision for entityB and tile
	for( int entityTypeID = 0; entityTypeID < (int)NUM_ENTITY_TYPES; entityTypeID++ )
	{
		EntityList& entityList = m_entityListsByType[entityTypeID];
		for( int entityID = 0; entityID < (int)entityList.size(); entityID++ )
		{
			Entity* entity = entityList[entityID];
			if( entity != nullptr && entity->IsAlive())
				ResolveEntityTileCollision( entity );
		}
	}
}

void Map::DetectCollisionForEntities()
{
	//detect collision for entities
	for( int entityTypeAID = 0; entityTypeAID < (int)ENTITY_TYPE_EXPLOSION; entityTypeAID++ )
	{
		EntityList& entityListA = m_entityListsByType[entityTypeAID];
		//pick up 
		if(entityTypeAID==ENTITY_TYPE_PICKUP )
			continue;

		//bullets
		else if( entityTypeAID == ENTITY_TYPE_EVIL_BULLET )
		{
			DetectCollisionForBulletList( entityListA, FACTION_EVIL );
		}
		else if( entityTypeAID == ENTITY_TYPE_GOOD_BULLET )
			DetectCollisionForBulletList( entityListA, FACTION_GOOD );

		//debug physics option
		else if( !g_isPhysicsEnabled && entityTypeAID == ENTITY_TYPE_PLAYER )
			continue;

		//Discuss collision for each entityA and other entities
		for( int entityAID = 0; entityAID < (int)entityListA.size(); entityAID++ )
		{
			for( int entityTypeBID = 0; entityTypeBID < (int)ENTITY_TYPE_EXPLOSION; entityTypeBID++ )
			{
				//bullets & pickups 
				if( entityTypeBID == ENTITY_TYPE_EVIL_BULLET || entityTypeBID == ENTITY_TYPE_GOOD_BULLET 
					|| entityTypeBID==ENTITY_TYPE_PICKUP )
					continue;

				//debug physics option
				if( !g_isPhysicsEnabled && entityTypeBID == ENTITY_TYPE_PLAYER )
					continue;

				EntityList& entityListB = m_entityListsByType[entityTypeBID];
				for( int entityBID = 0; entityBID < (int)entityListB.size(); entityBID++ )
				{
					//same entityB
					if( entityTypeAID == entityTypeBID && entityAID == entityBID )
						continue;
					Entity* entityA = entityListA[entityAID];
					Entity* entityB = entityListB[entityBID];
					if( entityA == nullptr || !entityA->IsAlive() || entityB == nullptr || !entityB->IsAlive() )
						continue;
					if( entityTypeAID == ENTITY_TYPE_BOMB || entityTypeBID == ENTITY_TYPE_BOMB )
					{
						if( entityA->m_faction != entityB->m_faction &&
							DoDiscsOverlap2D(entityA->m_position,entityA->m_physicsRadius,entityB->m_position,entityB->m_physicsRadius))
						{
							if( entityTypeAID == ENTITY_TYPE_BOMB )
							{
								entityA->Die();
							}
							if( entityTypeBID == ENTITY_TYPE_BOMB )
							{
								entityB->Die();
							}
						}
					}
					else ResolveEntitiesCollision( entityA, entityB );
				}
			}
		}
	}
}

void Map::DetectCollisionForBombs()
{
	EntityList& bombList = m_entityListsByType[ENTITY_TYPE_BOMB];
	for( int bID = 0; bID < (int)bombList.size(); bID++ )
	{
		Entity* bomb = bombList[bID];
		if( bomb == nullptr || !bomb->IsAlive() )
			continue;
	}
}

void Map::DetectCollisionForPickups()
{
	EntityList& pickupList = m_entityListsByType[ENTITY_TYPE_PICKUP];
	for( int pID = 0; pID < (int)pickupList.size(); pID++ )
	{
		Pickup*pickup = (Pickup*)pickupList[pID];
		if(pickup==nullptr||!pickup->IsAlive() )
			continue;
		DetectPickupCollisionForEntityType(pickup, ENTITY_TYPE_PLAYER );
		DetectPickupCollisionForEntityType( pickup, ENTITY_TYPE_NPC_TANK );
	}
}

void Map::DetectPickupCollisionForEntityType( Pickup* pickup, EntityType type )
{
	if( pickup == nullptr || !pickup->IsAlive() )
		return;
	EntityList& entityList = m_entityListsByType[type];
	for( int eID = 0; eID < (int)entityList.size(); eID++ )
	{
		Entity* entity = entityList[eID];
		if(entity==nullptr||!entity->IsAlive() )
			continue;
		if( DoDiscsOverlap2D( pickup->m_position, pickup->m_physicsRadius, entity->m_position, entity->m_physicsRadius ) )
		{
			if(entity->m_faction == pickup->m_faction)
				entity->PickupStuff( pickup->m_pickupType );
			pickup->Die();
		}
	}
}

void Map::DetectCollisionForBulletList(EntityList& bulletList,EntityFaction faction)
{
	for( int bID = 0; bID < (int)bulletList.size(); bID++ )
	{
		Entity* bullet = bulletList[bID];
		if( bullet == nullptr || !bullet->IsAlive())
			continue;
		for( int listID = 0; listID < ENTITY_TYPE_EXPLOSION; listID++ )
		{
			if(listID==ENTITY_TYPE_EVIL_BULLET||listID==ENTITY_TYPE_GOOD_BULLET ) //do not consider collision between bullets
				continue;
			if( !g_isPhysicsEnabled && listID == ENTITY_TYPE_PLAYER )
				continue;
			EntityList& thisList = m_entityListsByType[listID];
			for( int eID = 0; eID < (int)thisList.size(); eID++ )
			{
				Entity* entity = thisList[eID];
				if( entity != nullptr && entity->m_faction != faction && bullet!=entity
					&& DoDiscsOverlap2D(bullet->m_position,bullet->m_physicsRadius,entity->m_position,entity->m_physicsRadius))
				{
					if( listID == ENTITY_TYPE_BOULDER )
					{
						DeflectEntityOffEntity(bullet,entity);
						continue;//deflects
					}
					
					if( entity->m_isHitByBullets )
					{
						bullet->TakeDamage( 1 );
						entity->TakeDamage( 1 );
					}
				}
			}
		}
	}

}

void Map::ResolveFactionBombForEntityType( EntityType type, EntityFaction faction, const Vec2& position, float radius )
{
	EntityList& entityList = m_entityListsByType[type];
	for( int eID = 0; eID < (int)entityList.size(); eID++ )
	{
		Entity* entity = entityList[eID];
		if(entity==nullptr||!entity->IsAlive()||entity->m_faction==faction )
			continue;
		if((entity->m_position-position).GetLength()<radius )
			entity->SwitchFaction();
	}
}

void Map::ResolveTurretsOverlap()
{
	EntityList& turretList = m_entityListsByType[ENTITY_TYPE_NPC_TURRET];
	for( int turretA = 0; turretA < (int)turretList.size(); turretA++ )
	{
		ResolveOneTurretOverlap( turretList[turretA] );
	}
}

void Map::ResolveOneTurretOverlap( Entity* turret )
{
	EntityList turretList = m_entityListsByType[ENTITY_TYPE_NPC_TURRET];
	for( int tID = 0; tID < (int)turretList.size(); tID++ )
	{
		Entity* otherTurret = turretList[tID];
		if(turret== otherTurret)
			continue;
		PushDiscsOutOfEachOther2D( turret->m_position, turret->m_physicsRadius, otherTurret->m_position, otherTurret->m_physicsRadius );
	}
}

void Map::ResolveEntitiesCollision( Entity* entityA, Entity* entityB )
{
	if( !entityA->m_isPushedByEntities && !entityB->m_isPushedByEntities )//both can not be pushed
		return;

	if( !entityA->m_pushesEntities && !entityB->m_pushesEntities )//both can not push
		return;

	if( !DoDiscsOverlap2D( entityA->m_position, entityA->m_physicsRadius, entityB->m_position, entityB->m_physicsRadius ) )//do not overlap
		return;

	if( entityA->m_isPushedByEntities && entityB->m_isPushedByEntities )//push out of each other
	{
		PushDiscsOutOfEachOther2D( entityA->m_position, entityA->m_physicsRadius, entityB->m_position, entityB->m_physicsRadius );
		return;
	}
	if( entityA->m_isPushedByEntities && entityB->m_pushesEntities)//push a out of b
	{
		PushDiscOutOfDisc2D( entityA->m_position, entityA->m_physicsRadius, entityB->m_position, entityB->m_physicsRadius );
		return;
	}
	if( entityB->m_isPushedByEntities && entityA->m_pushesEntities )//push b out of a
	{
		PushDiscOutOfDisc2D( entityB->m_position, entityB->m_physicsRadius, entityA->m_position, entityA->m_physicsRadius );
	}

}

void Map::ResolveEntityTileCollision( Entity* entity )
{
	if( !entity->m_isPushedByWalls )
		return;

	IntVec2 searchOrder[8] = {
		IntVec2( 1,0 ),IntVec2( 0,1 ),IntVec2( -1,0 ),IntVec2( 0,-1 ),
		IntVec2( 1,1 ),IntVec2( -1,1 ),IntVec2( -1,-1 ),IntVec2( 1,-1 )
	};
	float radius = entity->m_physicsRadius;
	IntVec2 posCoords = GetTileCoordsForPosition( entity->m_position );
	//Assume that entityB radius < 1
	for( int searchID = 0; searchID < 8; searchID++ )
	{
		int tileID = GetTileIndexForTileCoords( posCoords + searchOrder[searchID] );
		if(tileID>=m_size.x*m_size.y || tileID<0)
			continue;

		Tile& tile = m_tiles[tileID];
		if( TileDefinition::s_definitions[tile.m_type].m_isSolid )
			PushDiscOutOfAABB2D( entity->m_position, radius, tile.GetBounds() );
	}
}

void Map::DeflectEntityOffEntity( Entity* entityMobile, Entity* entityStill )
{
	Vec2 normal = entityStill->m_position - entityMobile->m_position;
	const Vec2 velocityNormal = GetProjectedOnto2D( entityMobile->m_velocity, normal );
	Vec2 velocityTangent = entityMobile->m_velocity - velocityNormal;
	entityMobile->m_velocity = velocityTangent - velocityNormal;
	entityMobile->m_orientationDegrees = entityMobile->m_velocity.GetAngleDegrees();
	PushDiscOutOfDisc2D( entityMobile->m_position, entityMobile->m_physicsRadius, entityStill->m_position, entityStill->m_physicsRadius );
}

void Map::Render() const
{
	RenderTiles();
	RenderEntities();
	if( g_isDebugDrawing )
		DebugRender();
}

void Map::DebugRender() const
{
	g_theRenderer->BindDiffuseTexture( (Texture*)nullptr );
	for( int entityTypeID = 0; entityTypeID < (int)NUM_ENTITY_TYPES; entityTypeID++ )
	{
		EntityList entityList = m_entityListsByType[entityTypeID];
		for( int entityID = 0; entityID < (int)entityList.size(); entityID++ )
		{
			Entity* entity = entityList[entityID];
			if(entity!=nullptr )
				entity->DebugRender();
		}
	}
}

void Map::RenderTiles() const
{
	g_theRenderer->BindDiffuseTexture( (Texture*)nullptr );
	//could change to different tile types and different loops
	for( int tileTypeID = 0; tileTypeID < NUM_TILE_TYPE; tileTypeID++ )
	{
		g_tileVerts.clear();
		for( int tID = 0; tID < (int)m_tiles.size(); tID++ )
		{
			if(m_tiles[tID].m_type==(TileType)tileTypeID)
				m_tiles[tID].Render();
		}
		if(g_tileVerts.size()<1 )
			continue;

		const Texture& thisTexture = TileDefinition::s_definitions[(int)tileTypeID].m_texture;
		g_theRenderer->BindDiffuseTexture( &thisTexture );
		g_theRenderer->DrawVertexArray( g_tileVerts );
	}
}

void Map::RenderEntities() const
{
	for( int entityTypeID = 0; entityTypeID < (int)NUM_ENTITY_TYPES; entityTypeID++ )
	{
		EntityList entityList = m_entityListsByType[entityTypeID];
		if( entityTypeID == (int)NUM_ENTITY_TYPES - 1 )
			g_theRenderer->SetBlendMode( eBlendMode::BLEND_ADDITIVE );
		for( int entityID = 0; entityID < (int)entityList.size(); entityID++ )
		{
			Entity* entity = entityList[entityID];
			if( entity != nullptr && entity->IsAlive())
				entity->Render();
		}
		if( entityTypeID == (int)NUM_ENTITY_TYPES - 1 )
			g_theRenderer->SetBlendMode( eBlendMode::BLEND_ALPHA );
	}
}
