#include "Game/NpcTank.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Audio/AudioSystem.hpp"

//////////////////////////////////////////////////////////////////////////
NpcTank::NpcTank( Map* map, const Vec2& startPos, EntityFaction faction, EntityType type )
	:Entity(map,startPos,faction,type)
{
	m_health = NPC_TANK_HEALTH;
	m_healthLimit = m_health;

	m_isPushedByWalls = true;
	m_pushesEntities = true;
	m_isPushedByEntities = true;
	m_isHitByBullets = true;

	m_physicsRadius = NPC_TANK_PHYSICS_RADIUS;
	m_cosmeticRadius = NPC_TANK_COSMETIC_RADIUS;
	m_speedLimit = NPC_TANK_SPEED;

	float halfBaseSize = m_cosmeticRadius;
	AABB2 bounds( Vec2( -halfBaseSize, -halfBaseSize ), Vec2( halfBaseSize, halfBaseSize ) );
	AppendVertsForAABB2D( m_verts, bounds );
}

//////////////////////////////////////////////////////////////////////////
void NpcTank::Update( float deltaSeconds )
{
	if( !IsAlive() )
		return;

	//check if see enemy
	Entity* visibleEnemy = m_theMap->RaycastForEnemyFaction( m_faction, m_position, NPC_TANK_DETECT_LENGTH );
	if( visibleEnemy!=nullptr )//can see enemy
	{
		m_goalPosReached = false;
		m_goalAngleReached = true;
		m_goalPos = visibleEnemy->m_position;
		CheckToShoot(deltaSeconds);		
	}
	else //can't see enemy
	{
		//check if see pickup
		EntityFaction oppoFaction = m_faction == FACTION_GOOD ? FACTION_EVIL : FACTION_GOOD;
		Entity* visiblePickup = m_theMap->RaycastForEnemyType( oppoFaction, ENTITY_TYPE_PICKUP, m_position, NPC_TANK_DETECT_LENGTH );
		if( visiblePickup != nullptr )
		{
			m_goalPosReached = false;
			m_goalPosReached = true;
			m_goalPos = visiblePickup->m_position;
		}
	}
	
	//move toward goal position
	if( !m_goalPosReached )
	{
		UpdateForGoalPosNotReached();
	}
	//Revise orientation to avoid prolonged collision with solid tiles
	if( m_goalAngleReached )
	{
		UpdateWhiskerDetection();
	}
	//no goal position, turn to randomized goal orientation
	if(m_goalPosReached)
	{
		UpdateForGoalPosReached( deltaSeconds );
	}
	//check if reach goal angle
	if( m_goalOrientation == m_orientationDegrees )
		m_goalAngleReached = true;
	
	//set orientation
	m_orientationDegrees = GetTurnedToward( m_orientationDegrees, m_goalOrientation, NPC_TANK_TURN_SPEED * deltaSeconds );
	//set velocity
	float deltaDegreesToGoal = m_goalOrientation - m_orientationDegrees;
	if( deltaDegreesToGoal > NPC_TANK_FORWARD_DEGREES || deltaDegreesToGoal < -NPC_TANK_FORWARD_DEGREES )
		m_velocity = .4f * m_speedLimit * Vec2::MakeFromPolarDegrees( m_orientationDegrees );
	else m_velocity = m_speedLimit * Vec2::MakeFromPolarDegrees( m_orientationDegrees );
	//update
	Entity::Update( deltaSeconds );
}

//////////////////////////////////////////////////////////////////////////
void NpcTank::Render() const
{	
	Texture* tankTexture =nullptr;
	if( m_faction == FACTION_EVIL )
		tankTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/EnemyTank4.png" );
	else if( m_faction == FACTION_GOOD )
		tankTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/FriendlyTank4.png" );
	g_theRenderer->BindDiffuseTexture( tankTexture );
	Entity::Render();

	//health bar
	g_theRenderer->BindDiffuseTexture( (Texture*)nullptr );
	Vec2 healthBarLeft = m_position + Vec2( -HEALTH_BAR_LENGTH*.5f, m_cosmeticRadius );
	float healthRate = (float)m_health / (float)NPC_TANK_HEALTH;
	g_theRenderer->DrawLine2D( healthBarLeft, healthBarLeft + Vec2( HEALTH_BAR_LENGTH*healthRate, 0.f ), 5 * LINE_THICKNESS, Rgba8( 255, 0, 0 ) );
}

//////////////////////////////////////////////////////////////////////////
void NpcTank::DebugRender() const
{
	Entity::DebugRender();

	if( !m_goalPosReached )
	{
		g_theRenderer->DrawLine2D( m_position, m_goalPos, LINE_THICKNESS, Rgba8( 255, 0, 0 ) );
		g_theRenderer->DrawDisc2D( m_goalPos, .2f, Rgba8( 255, 0, 0 ) );
	}

	//whiskers
	Vec2 forward = Vec2::MakeFromPolarDegrees( m_orientationDegrees );
	Vec2 side = forward.GetRotated90Degrees();
	g_theRenderer->DrawLine2D( m_position + side * m_physicsRadius, m_leftWhiskerResult.m_impactPos, LINE_THICKNESS, Rgba8::WHITE );
	g_theRenderer->DrawLine2D( m_position - side * m_physicsRadius, m_rightWhiskerResult.m_impactPos, LINE_THICKNESS, Rgba8::WHITE );
	g_theRenderer->DrawLine2D( m_position,                          m_centerWhiskerResult.m_impactPos, LINE_THICKNESS, Rgba8::WHITE );
}

//////////////////////////////////////////////////////////////////////////
void NpcTank::TakeDamage( int damage )
{
	Entity::TakeDamage( damage );

	SoundID hitSound = g_theAudio->CreateOrGetSound( "Data/Audio/EnemyHit.wav" );
	g_theAudio->PlaySound( hitSound );
}

//////////////////////////////////////////////////////////////////////////
void NpcTank::Die()
{
	Entity::Die();

	m_theMap->SpawnExplosion( m_position, m_cosmeticRadius, .5f*EXPLOSION_MAX_DURATION );

	EntityFaction newFaction = GetOppositeFaction();
	m_theMap->SpawnPickup( newFaction,m_position );

	SoundID dieSound = g_theAudio->CreateOrGetSound( "Data/Audio/EnemyDied.wav" );
	g_theAudio->PlaySound( dieSound );
}

//////////////////////////////////////////////////////////////////////////
void NpcTank::UpdateForGoalPosNotReached()
{
	Vec2 tankToPlayer = (m_goalPos - m_position);
	if( tankToPlayer.GetLength() < m_physicsRadius )
	{
		m_goalPosReached = true;
	}
	else
	{
		m_goalOrientation = tankToPlayer.GetAngleDegrees();
	}
}

//////////////////////////////////////////////////////////////////////////
void NpcTank::UpdateForGoalPosReached( float deltaSeconds )
{
	if( m_resetGoalOrienCountdown <= 0.f )
	{
		m_goalOrientation = g_theGame->m_RNG->RollRandomFloatInRange( 0.f, 360.f );
		m_resetGoalOrienCountdown = NPC_TANK_TURN_COUNTDOWN;
	}
	else m_resetGoalOrienCountdown -= deltaSeconds;
}

//////////////////////////////////////////////////////////////////////////
void NpcTank::UpdateWhiskerDetection()
{
	Vec2 forward = Vec2::MakeFromPolarDegrees( m_orientationDegrees );
	Vec2 side = forward.GetRotated90Degrees();
	m_leftWhiskerResult = m_theMap->Raycast( m_position + side * m_physicsRadius, forward, 1.f );
	m_rightWhiskerResult = m_theMap->Raycast( m_position - side * m_physicsRadius, forward, 1.f );
	m_centerWhiskerResult = m_theMap->Raycast( m_position, forward, 1.f );

	if( m_leftWhiskerResult.m_impacted || m_rightWhiskerResult.m_impacted || m_centerWhiskerResult.m_impacted )
	{		
		if( m_leftWhiskerResult.m_impacted && m_rightWhiskerResult.m_impacted && m_centerWhiskerResult.m_impacted )//stuck in corner
		{
			m_goalOrientation = -m_orientationDegrees;
			m_goalAngleReached = false;
		}
		else if( m_leftWhiskerResult.m_impacted && !m_rightWhiskerResult.m_impacted ) //should turn right
		{
			m_goalOrientation = m_leftWhiskerResult.m_impactNormal.GetAngleDegrees() + 70.f;
		}
		else if( m_rightWhiskerResult.m_impacted && !m_leftWhiskerResult.m_impacted ) // should turn left
		{
			m_goalOrientation = m_rightWhiskerResult.m_impactNormal.GetAngleDegrees() - 70.f;
		}
		//both stuck
		else
		{
			float m_leftWhiskerRevised = m_leftWhiskerResult.m_impactNormal.GetAngleDegrees() + 70.f;
			float m_rightWhiskerRevised = m_rightWhiskerResult.m_impactNormal.GetAngleDegrees() - 70.f;
			if( m_goalPosReached )
			{
				if( m_leftWhiskerResult.m_impactDist < m_rightWhiskerResult.m_impactDist )
				{
					m_goalOrientation = m_leftWhiskerRevised;
				}
				else
				{
					m_goalOrientation = m_rightWhiskerRevised;
				}
			}
			else
			{
				float tankToPlayerDegrees = (m_goalPos - m_position).GetAngleDegrees();
				//right better
				if( IsAbsValueBigger( GetShortestAngularDisplacement(m_leftWhiskerRevised,tankToPlayerDegrees), 
					GetShortestAngularDisplacement(m_rightWhiskerRevised, tankToPlayerDegrees ) ))
				{
					m_goalOrientation = m_rightWhiskerRevised;
				}
				else m_goalOrientation = m_leftWhiskerRevised;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void NpcTank::CheckToShoot(float deltaSeconds)
{
	float tankToPlayerDegrees = (m_goalPos - m_position).GetAngleDegrees();
	float deltaDegreesToPlayer = tankToPlayerDegrees - m_orientationDegrees;
	if( deltaDegreesToPlayer<NPC_TANK_SHOOT_DEGREES && deltaDegreesToPlayer>-NPC_TANK_SHOOT_DEGREES )//could shoot
	{
		if( m_shootCountdown <= 0.f )
		{
			ShootBullet();
			m_shootCountdown = NPC_TANK_SHOOT_COOLDOWN;
		}
		else
		{
			m_shootCountdown -= deltaSeconds;
		}
	}
	else
	{
		m_shootCountdown = 0.f;
	}
}

//////////////////////////////////////////////////////////////////////////
void NpcTank::ShootBullet()
{
	Vec2 spawnPos = m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees )*m_physicsRadius;

	if( m_factionBombNum > 0 )
	{
		m_theMap->SpawnBomb( m_faction, spawnPos, m_orientationDegrees );
		m_factionBombNum--;
	}
	else if( m_faction == FACTION_EVIL )
		m_theMap->SpawnBullet( ENTITY_TYPE_EVIL_BULLET, m_faction, spawnPos, m_orientationDegrees );
	else if( m_faction == FACTION_GOOD )
		m_theMap->SpawnBullet( ENTITY_TYPE_GOOD_BULLET, m_faction, spawnPos, m_orientationDegrees );

	SoundID shootSound = g_theAudio->CreateOrGetSound( "Data/Audio/EnemyShoot.wav" );
	g_theAudio->PlaySound( shootSound );
}
