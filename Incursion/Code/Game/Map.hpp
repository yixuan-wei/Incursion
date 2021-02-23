#pragma once

#include <vector>
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"
#include "Game/WormDefinition.hpp"

class Tile;
class Game;
class World;
class Entity;
class Pickup;
struct Vertex_PCU;
enum TileType : int;

extern std::vector<Vertex_PCU> g_tileVerts;

struct RaycastResult
{
	bool m_impacted;
	Vec2 m_impactPos;
	float m_impactDist;
	Vec2 m_impactNormal;
	TileType m_impactTileType;
	//Entity* m_impactEntity;
};

class Map
{
	friend class World;
	friend class Game;

public:
	Map(Game* game, World* world, const IntVec2& tileDimension);
	~Map();

	void StartUp(int turretNum, int tankNum, int boulderNum);

	Entity* SpawnNewEntity( EntityType type, EntityFaction faction, const Vec2& spawnPosition );
	Entity* SpawnBullet( EntityType type, EntityFaction faction, const Vec2& spawnPos, float orientation );
	Entity* SpawnBomb( EntityFaction faction, const Vec2& spawnPos, float orientation );
	Entity* SpawnPlayer( EntityFaction faction, const Vec2& preferSpawnPosition );
	Entity* SpawnExplosion( const Vec2& spawnPosition, float radius, float duration, const Rgba8& tint = Rgba8::WHITE );
	Entity* SpawnPickup( EntityFaction faction, const Vec2& spawnPosition );
	Entity* SpawnNPC( EntityType type, EntityFaction faction );
	void    AddEntityToMap( Entity* entity );
	void    AddEntityToList( Entity* entity, EntityList& entityList );

	void   ResolveFactionBombExlopsion( EntityFaction faction, const Vec2& position, float radius );

	int     GetTileIndexForTileCoords( const IntVec2& tileCoords ) const;
	int     GetTileIndexForPosition( const Vec2& position ) const;
	IntVec2 GetTileCoordsForTileIndex( int tileIndex ) const;
	IntVec2 GetTileCoordsForPosition( const Vec2& position ) const;
	Entity* GetPlayerAlive() const;

	bool IsPointInSolid( const Vec2& point ) const;
	bool IsPointInTileType( const Vec2& point, TileType type )const;
	bool IsTileSolid( const IntVec2& tileCoords ) const;
	bool IsTileInEdge( const IntVec2& tileCoords ) const;
	bool HasLineOfSight( const Vec2& startPoint, const Vec2& endPoint, float maxDist ) const;
	RaycastResult Raycast( const Vec2& startPosition, const Vec2& forwardDir, float maxDist ) const;	
	Entity* RaycastForEnemyFaction( EntityFaction faction, const Vec2& startPoint, float maxDist ) const;
	Entity* RaycastForEnemyType( EntityFaction faction, EntityType type, const Vec2& startPoint, float maxDist ) const;
	Entity* RaycastForEnemyEntity( EntityFaction faction, Entity* enemy, const Vec2& startpoint, float maxDist )const;

private:
	World*  m_world = nullptr;
	Game* m_game = nullptr;
	IntVec2 m_size;
	float m_playerRespawnCountdown = PLAYER_RESPAWN_INTERVAL;
	std::vector<Tile> m_tiles;
	EntityList m_entityListsByType[NUM_ENTITY_TYPES];

	void GenerateMap( TileType defaultTile, TileType edgeTile, TileType startTile, TileType endTile, std::vector<WormDefinition>& wormDefs );
	void InitTiles( TileType defaultTile, TileType edgeTile, TileType startTile, TileType endTile, std::vector<WormDefinition>& wormDefs );
	void InitWormsForDefinition( WormDefinition wormDef );

	IntVec2 GetRandomAdjacentTileCoords( const IntVec2& tileCoords );
	Vec2    GetEnemySpawnPoint() const;
	float   GetTileSpeedFactorForPoint( const Vec2& point ) const;

	bool IsMapWalkable();
	bool IsLevelCompleted() const;
	bool IsEnemyFactionExist(EntityFaction faction) const;

	void Update( float deltaSeconds );
	void UpdateEntities( float deltaSeconds );
	void ClearEntities();
	void CleanDeadTrashEntities();

	void DetectCollisionForTilesAndEntities();
	void DetectCollisionForEntities();
	void DetectCollisionForBombs();
	void DetectCollisionForPickups();
	void DetectPickupCollisionForEntityType( Pickup* pickup, EntityType type );
	void DetectCollisionForBulletList(EntityList& bulletList, EntityFaction faction);
	void ResolveFactionBombForEntityType( EntityType type, EntityFaction faction, const Vec2& position, float radius );
	void ResolveTurretsOverlap();
	void ResolveOneTurretOverlap( Entity* turret );
	void ResolveEntitiesCollision( Entity* entityA, Entity* entityB );
	void ResolveEntityTileCollision( Entity* entity );
	void DeflectEntityOffEntity( Entity* entityMobile, Entity* entityStill );

	void Render()const;
	void DebugRender()const;
	void RenderTiles()const;
	void RenderEntities()const;
};