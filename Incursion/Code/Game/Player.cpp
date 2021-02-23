#include "Game/Player.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Input/XboxController.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Audio/AudioSystem.hpp"

//////////////////////////////////////////////////////////////////////////
Player::Player( Map* map, const Vec2& startPos, EntityFaction faction, EntityType type )
	:Entity(map, startPos,faction, type)
	,m_controllerID(0)
{
	m_speedLimit = PLAYER_SPEED;
	m_physicsRadius = PLAYER_PHYSICS_RADIUS;
	m_cosmeticRadius = PLAYER_COSMETIC_RADIUS;

	m_pushesEntities = true;
	m_isPushedByEntities = true;
	m_isHitByBullets = true;
	m_isPushedByWalls = true;

	m_health = PLAYER_HEALTH;
	m_healthLimit = m_health;

	float halfBaseSize = m_cosmeticRadius;
	AABB2 bounds( Vec2( -halfBaseSize, -halfBaseSize ), Vec2( halfBaseSize, halfBaseSize ) );
	AppendVertsForAABB2D( m_verts, bounds );
	
}

//////////////////////////////////////////////////////////////////////////
void Player::Update( float deltaSeconds )
{
	if( m_vibrationCounter > 0 )
	{
		m_vibrationCounter -= deltaSeconds;
	}
	else
	{
		g_theInput->SetVibrationValue( m_controllerID, 0, 0 );
	}

	if( !IsAlive() )
		return;

	m_thrustFraction = 0.f;
	UpdateFromController(deltaSeconds);

	m_velocity = Vec2( 0.f, 0.f );
	if( m_thrustFraction > 0.f )
	{
		m_velocity = Vec2::MakeFromPolarDegrees( m_orientationDegrees, m_thrustFraction );
	}

	Entity::Update( deltaSeconds );
}

//////////////////////////////////////////////////////////////////////////
void Player::Render() const
{
	//base
	Texture* baseTank = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/PlayerTankBase.png" );
	g_theRenderer->BindDiffuseTexture( baseTank );
	Entity::Render();

	//turret
	Texture* turretTank = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/PlayerTankTop.png" );
	g_theRenderer->BindDiffuseTexture( turretTank );
	std::vector<Vertex_PCU> turretVerts = m_verts;
	TransformVertexArray( (int)turretVerts.size(), &turretVerts[0], 1.f, m_orientationDegrees + m_gunRelativeOrientation, m_position );
	g_theRenderer->DrawVertexArray( turretVerts );

	//health bar
	g_theRenderer->BindDiffuseTexture((Texture*)nullptr );
	Vec2 healthBarLeft = m_position + Vec2( -HEALTH_BAR_LENGTH * .5f, m_cosmeticRadius );
	float healthRate = (float)m_health / (float)PLAYER_HEALTH;
	g_theRenderer->DrawLine2D( healthBarLeft, healthBarLeft + Vec2( HEALTH_BAR_LENGTH * healthRate, 0.f ), 5 * LINE_THICKNESS, Rgba8( 255, 0, 0 ) );
}

//////////////////////////////////////////////////////////////////////////
void Player::DebugRender() const
{
	Entity::DebugRender();

	Vec2 forward = Vec2::MakeFromPolarDegrees( m_orientationDegrees + m_gunRelativeOrientation );
	RaycastResult result = m_theMap->Raycast( m_position, forward, 100.f );
	g_theRenderer->DrawLine2D( m_position, result.m_impactPos, LINE_THICKNESS, Rgba8( 255, 0, 0 ) );
}

//////////////////////////////////////////////////////////////////////////
void Player::TakeDamage( int damage )
{
	Entity::TakeDamage( damage );

	SoundID hitSound = g_theAudio->CreateOrGetSound( "Data/Audio/PlayerHit.wav" );
	g_theAudio->PlaySound( hitSound );

	g_theInput->SetVibrationValue( m_controllerID, .3f, .3f );
	m_vibrationCounter = PLAYER_HIT_VIBRATION_TIME;
}

//////////////////////////////////////////////////////////////////////////
void Player::Die()
{
	Entity::Die();

	m_theMap->SpawnExplosion( m_position, 2.f*m_cosmeticRadius, EXPLOSION_MAX_DURATION );

	SoundID dieSound = g_theAudio->CreateOrGetSound( "Data/Audio/PlayerDied.wav" );
	g_theAudio->PlaySound( dieSound );
}

//////////////////////////////////////////////////////////////////////////
void Player::UpdateFromController(float deltaSeconds)
{
	if( m_controllerID < 0 )
		return;

	const XboxController controller = g_theInput->GetXboxController( m_controllerID );
	if( !controller.IsConnected() )
		return;

	//movement
	const AnalogJoystick leftJoystick = controller.GetLeftJoystick();
	float leftMagnitude = leftJoystick.GetMagnitude();
	if( leftMagnitude > 0.f )
	{
		m_thrustFraction = leftMagnitude;
		m_orientationDegrees = GetTurnedToward( m_orientationDegrees, leftJoystick.GetAngleDegrees(), PLAYER_TURN_SPEED * deltaSeconds );
	}

	//gun movement
	const AnalogJoystick rightJoystick = controller.GetRightJoystick();
	float magnitude = rightJoystick.GetMagnitude();
	if( magnitude > 0.f )
	{
		float turnedAbsolute = GetTurnedToward( m_orientationDegrees + m_gunRelativeOrientation, 
			rightJoystick.GetAngleDegrees(), PLAYER_GUN_TURN_SPEED * deltaSeconds );
		m_gunRelativeOrientation = turnedAbsolute - m_orientationDegrees;
	}

	//shoot bullet
	const KeyButtonState aButton = controller.GetButtonState( XBOX_BUTTON_ID_RSHOULDER );
	if( aButton.WasJustPressed() )
	{
		ShootBullet();
	}

	//shoot bomb
	const KeyButtonState bButton = controller.GetButtonState( XBOX_BUTTON_ID_LSHOULDER );
	if( bButton.WasJustPressed() )
	{
		ShootBomb();
	}
}

//////////////////////////////////////////////////////////////////////////
void Player::ShootBullet()
{
	float absoluteBulletOrientation = m_orientationDegrees + m_gunRelativeOrientation;
	m_theMap->SpawnBullet( ENTITY_TYPE_GOOD_BULLET, FACTION_GOOD, 
		m_position + Vec2::MakeFromPolarDegrees(absoluteBulletOrientation, m_cosmeticRadius ), absoluteBulletOrientation );
	
	SoundID shootSound = g_theAudio->CreateOrGetSound( "Data/Audio/PlayerShootNormal.ogg" );
	g_theAudio->PlaySound( shootSound );
}

//////////////////////////////////////////////////////////////////////////
void Player::ShootBomb()
{
	if( m_factionBombNum <= 0 )
		return;

	float absoluteBulletOrientation = m_orientationDegrees + m_gunRelativeOrientation;
	m_theMap->SpawnBomb( m_faction,
		m_position + Vec2::MakeFromPolarDegrees( absoluteBulletOrientation, m_cosmeticRadius ), absoluteBulletOrientation );
	
	SoundID shootSound = g_theAudio->CreateOrGetSound( "Data/Audio/PlayerShootNormal.ogg" );
	g_theAudio->PlaySound( shootSound );

	m_factionBombNum--;
}

