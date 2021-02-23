#include "Game/NpcTurret.hpp"
#include "Game/Map.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Audio/AudioSystem.hpp"

//////////////////////////////////////////////////////////////////////////
NpcTurret::NpcTurret( Map* map, const Vec2& startPos, EntityFaction faction, EntityType type )
	:Entity(map,startPos,faction,type)
{
	m_health = NPC_TURRET_HEALTH;
	m_healthLimit = m_health;
	m_isPushedByWalls = true;
	m_pushesEntities = true;
	m_isHitByBullets = true;
	m_physicsRadius = NPC_TURRET_PHYSICS_RADIUS;
	m_cosmeticRadius = NPC_TURRET_COSMETIC_RADIUS;
	m_angularVelocity = NPC_TURRET_TURN_SPEED;

	float halfBaseSize = m_cosmeticRadius;
	AABB2 bounds( Vec2( -halfBaseSize, -halfBaseSize ), Vec2( halfBaseSize, halfBaseSize ) );
	AppendVertsForAABB2D( m_verts, bounds );
}

//////////////////////////////////////////////////////////////////////////
void NpcTurret::Update( float deltaSeconds )
{
	if( !IsAlive() )
		return;

	// can see anti-faction enemy
	Entity* visibleEnemy = m_theMap->RaycastForEnemyFaction(m_faction,m_position,NPC_TURRET_DETECT_LENGTH);
	if(visibleEnemy!=nullptr )
	{
		m_enemySeen = true;
		m_angularVelocity = 0.f;
		m_impactedPos = visibleEnemy->m_position;
		m_lastEnemySeenDegrees = (m_impactedPos - m_position).GetAngleDegrees();
		m_orientationDegrees = GetTurnedToward( m_orientationDegrees, m_lastEnemySeenDegrees, NPC_TURRET_TURN_SPEED * deltaSeconds );
		//shoot
		if( m_lastEnemySeenDegrees - m_orientationDegrees > -NPC_TURRET_SHOOT_DEGREES && 
			m_lastEnemySeenDegrees - m_orientationDegrees < NPC_TURRET_SHOOT_DEGREES )
		{
			if( m_shootCountdown <= 0.f )
			{
				ShootBullet();
				m_shootCountdown = NPC_TURRET_SHOOT_COOLDOWN;
			}
			else m_shootCountdown -= deltaSeconds;
		}
		else m_shootCountdown = 0.f;
	}
	// can't see player
	else 
	{
		if( !m_enemySeen || m_angularVelocity==0.f ||
			m_orientationDegrees < m_lastEnemySeenDegrees - NPC_TURRET_SCAN_RANGE )//anti-clockwise rotate
		{
			m_angularVelocity = NPC_TURRET_TURN_SPEED;
		}
		else if( m_orientationDegrees > m_lastEnemySeenDegrees + NPC_TURRET_SCAN_RANGE )//back and forth from last seen direction
		{			
			m_angularVelocity = -NPC_TURRET_TURN_SPEED;
		}
		Vec2 forward = Vec2::MakeFromPolarDegrees( m_orientationDegrees );
		RaycastResult result = m_theMap->Raycast( m_position, forward, NPC_TURRET_DETECT_LENGTH );
		m_impactedPos = result.m_impactPos;
	}

	Entity::Update( deltaSeconds );
}

//////////////////////////////////////////////////////////////////////////
void NpcTurret::Render() const
{
	//draw base
	Texture* baseTexture = nullptr;
	if( m_faction == FACTION_EVIL )
		baseTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/EnemyTurretBase.png" );
	else if( m_faction == FACTION_GOOD )
		baseTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/FriendlyTurretBase.png" );
	g_theRenderer->BindDiffuseTexture( baseTexture );

	std::vector<Vertex_PCU> baseVerts = m_verts;
	TransformVertexArray( (int)baseVerts.size(), &baseVerts[0], 1.f, 0.f, m_position );
	g_theRenderer->DrawVertexArray( baseVerts );

	//draw laser
	g_theRenderer->BindDiffuseTexture( (Texture*)nullptr );
	g_theRenderer->DrawLine2D( m_position, m_impactedPos, LINE_THICKNESS, Rgba8( 255, 0, 0 ) );

	//draw turret
	Texture* topTexture = nullptr;
	if(m_faction==FACTION_EVIL )
		topTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/EnemyTurretTop.png" );
	else if(m_faction==FACTION_GOOD )
		topTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/FriendlyTurretTop.png" );
	g_theRenderer->BindDiffuseTexture( topTexture );

	Entity::Render();

	//health bar
	g_theRenderer->BindDiffuseTexture((Texture*)nullptr );
	Vec2 healthBarLeft = m_position + Vec2( -HEALTH_BAR_LENGTH * .5f, m_cosmeticRadius );
	float healthRate = (float)m_health / (float)NPC_TURRET_HEALTH;
	g_theRenderer->DrawLine2D( healthBarLeft, healthBarLeft + Vec2( HEALTH_BAR_LENGTH * healthRate, 0.f ), 5 * LINE_THICKNESS, Rgba8( 255, 0, 0 ) );
}

//////////////////////////////////////////////////////////////////////////
void NpcTurret::TakeDamage( int damage )
{
	Entity::TakeDamage( damage );

	SoundID hitSound = g_theAudio->CreateOrGetSound( "Data/Audio/EnemyHit.wav" );
	g_theAudio->PlaySound( hitSound );
}

//////////////////////////////////////////////////////////////////////////
void NpcTurret::Die()
{
	Entity::Die();

	m_theMap->SpawnExplosion( m_position, m_cosmeticRadius, .5f*EXPLOSION_MAX_DURATION );

	EntityFaction newFaction = GetOppositeFaction();
	m_theMap->SpawnPickup( newFaction,m_position );

	SoundID dieSound = g_theAudio->CreateOrGetSound( "Data/Audio/EnemyDied.wav" );
	g_theAudio->PlaySound( dieSound );
}

//////////////////////////////////////////////////////////////////////////
void NpcTurret::ShootBullet()
{
	Vec2 spawnPos = m_position + m_cosmeticRadius * Vec2::MakeFromPolarDegrees( m_orientationDegrees );
	if( m_faction == FACTION_EVIL )
		m_theMap->SpawnBullet( ENTITY_TYPE_EVIL_BULLET, m_faction, spawnPos, m_orientationDegrees );
	else if(m_faction==FACTION_GOOD )
		m_theMap->SpawnBullet( ENTITY_TYPE_GOOD_BULLET, m_faction, spawnPos, m_orientationDegrees );

	SoundID shootSound = g_theAudio->CreateOrGetSound( "Data/Audio/EnemyShoot.wav" );
	g_theAudio->PlaySound( shootSound );
}
