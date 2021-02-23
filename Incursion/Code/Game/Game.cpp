#include "Game/Game.hpp"
#include "Game/World.hpp"
#include "Game/App.hpp"
#include "Game/Map.hpp"
#include "Game/Entity.hpp"
#include "Game/TileDefinition.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Audio/AudioSystem.hpp"

void Game::Startup()
{
	m_gameClock = new Clock();
	g_theRenderer->SetupParentClock(m_gameClock);
	g_theInput->PushMouseOptions(eMousePositionMode::MOUSE_ABSOLUTE, false, false);
	g_theFont = g_theRenderer->CreateOrGetBitmapFont( "Data/Fonts/SquirrelFixedFont" );

	m_worldCamera = new Camera();
	m_worldCamera->SetClearMode(CLEAR_COLOR_BIT, Rgba8::BLACK, 1.f);
    Vec2 resolution = g_theApp->GetWindowDimensions();
    Vec2 halfSize = resolution * .5f;
    m_worldCamera->SetOrthoView(-halfSize, halfSize);
    m_worldCamera->SetProjectionOrthographic(CAMERA_VIEW_SIZE_Y);

    m_uiCamera = new Camera();
	m_uiCamera->SetOrthoView( -halfSize,halfSize );
	m_uiCamera->SetProjectionOrthographic(CAMERA_VIEW_SIZE_Y);
}

void Game::Shutdown()
{
	g_theInput->PopMouseOptions();

	delete m_theWorld;
	m_theWorld = nullptr;

	delete m_worldCamera;
	delete  m_uiCamera;
}

void Game::Update()
{	
	float deltaSeconds = (float)m_gameClock->GetLastDeltaSeconds();
	if( m_gameState == GAME_STATE_LOADING )
	{
		if( !m_loadingStarted )
			m_loadingStarted = true;
		else
		{
			LoadAssets();
			//generate new RNG, tile definitions, world
			m_RNG = new RandomNumberGenerator();
			TileDefinition::InitializeDefinitions();
			m_theWorld = new World( this );
			//to title stage
			m_gameState = GAME_STATE_TITLE;
		}
	}
	else if( m_gameState==GAME_STATE_PLAYING)
	{
		deltaSeconds *= m_timeScale;
		m_theWorld->Update( deltaSeconds );
		UpdateCamera(deltaSeconds);
		UpdateForPlayerDeath(deltaSeconds);
	}
	else if( m_gameState == GAME_STATE_PAUSE )
	{
		TogglePauseState();
	}
	else if( m_gameState == GAME_STATE_TITLE )
	{
		UpdateForTitle();
	}
	else if(m_gameState==GAME_STATE_LOSE )
	{
		//when lose, transit to start automatically
		m_sceneCountdown -= deltaSeconds;
		if( m_sceneCountdown <= 0 )
		{
			m_gameState = GAME_STATE_TITLE;
		}
	}
	else if(m_gameState==GAME_STATE_WIN )
	{
		if( m_sceneCountdown > 0 )
		{
			m_sceneCountdown -= deltaSeconds;
			m_alphaCountup += deltaSeconds;
		}
		else
		{
			UpdateForWin();
		}
	}
}

void Game::Render() const
{	
		//world camera
    g_theRenderer->BeginCamera(m_worldCamera); 
    if (m_gameState != GAME_STATE_TITLE && m_gameState != GAME_STATE_LOADING)
    {
        m_theWorld->Render();
    }
	g_theRenderer->EndCamera( m_worldCamera );
		//ui camera
    g_theRenderer->BeginCamera(m_uiCamera);
    if (m_gameState != GAME_STATE_TITLE && m_gameState != GAME_STATE_LOADING)
    {
        RenderUIForPlay();
    }
	RenderUITitle();
    g_theRenderer->EndCamera(m_uiCamera);	
}

bool Game::IsInPlayState() const
{
	if( m_gameState==GAME_STATE_PLAYING )
		return true;
	else return false;
}

void Game::ProgressToState( GameState nextState )
{
	m_gameState = nextState;
	if( m_gameState == GAME_STATE_WIN )
	{
		m_sceneCountdown = 1.f;
		m_alphaCountup = 0.f;
	}
}

void Game::TogglePauseState()
{	
	if( g_theInput->WasKeyJustPressed( 'P' ) )
	{
		SetPauseState();
	}
	const XboxController controller = g_theInput->GetXboxController( 0 );
	if( !controller.IsConnected() )
		return;
	const KeyButtonState startButton = controller.GetButtonState( XBOX_BUTTON_ID_START );
	if( startButton.WasJustPressed() )
		SetPauseState();
	if( m_gameState == GAME_STATE_PAUSE )
	{
		if( g_theInput->WasKeyJustPressed( KEY_ESC ) )
			m_gameState = GAME_STATE_TITLE;
		const KeyButtonState endButton = controller.GetButtonState( XBOX_BUTTON_ID_BACK );
		if( endButton.WasJustPressed() )
			m_gameState = GAME_STATE_TITLE;
	}
}

void Game::LoadAssets()
{
	//load assets
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Extras_4x4.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Terrain_8x8.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/EnemyBullet.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/FriendlyBullet.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/EnemyTank4.png" ); 
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/FriendlyTank4.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/EnemyTurretBase.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/FriendlyTurretBase.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/EnemyTurretTop.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/FriendlyTurretTop.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/PlayerTankBase.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/PlayerTankTop.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Explosion_5x5.png" );
	g_theAudio->CreateOrGetSound( "Data/Audio/PlayerHit.wav" );
	g_theAudio->CreateOrGetSound( "Data/Audio/PlayerShootNormal.ogg" );
	g_theAudio->CreateOrGetSound( "Data/Audio/PlayerDied.wav" );
	g_theAudio->CreateOrGetSound( "Data/Audio/EnemyShoot.wav" );
	g_theAudio->CreateOrGetSound( "Data/Audio/EnemyDied.wav" );
	g_theAudio->CreateOrGetSound( "Data/Audio/EnemyHit.wav" );
	g_theAudio->CreateOrGetSound( "Data/Audio/Pause.mp3" );
	g_theAudio->CreateOrGetSound( "Data/Audio/Unpause.mp3" );
}

void Game::SetPauseState()
{
	if( m_gameState == GAME_STATE_PAUSE )
	{
		m_gameState = m_lastGameState;
		SoundID unpause = g_theAudio->CreateOrGetSound( "Data/Audio/Unpause.mp3" );
		g_theAudio->PlaySound( unpause );
	}
	else
	{
		m_lastGameState = m_gameState;
		m_gameState = GAME_STATE_PAUSE;
		SoundID pause = g_theAudio->CreateOrGetSound( "Data/Audio/Pause.mp3" );
		g_theAudio->PlaySound( pause );
	}
}

void Game::UpdateEventStates()
{
	//F1 draw debug render
	if( g_theInput->WasKeyJustPressed( KEY_F1 ) )
		g_isDebugDrawing = !g_isDebugDrawing;
	//F3 physics switch
	if( g_theInput->WasKeyJustPressed( KEY_F3 ) )
		g_isPhysicsEnabled = !g_isPhysicsEnabled;
	//F4 full screen map mode
	if( g_theInput->WasKeyJustPressed( KEY_F4 ) )
		g_isFullScreenMap = !g_isFullScreenMap;
	//slow down
	if( g_theInput->IsKeyDown( 'T' ) && m_timeScale == 1.f )
	{
		m_timeScale = .1f;
	}
	else if( g_theInput->WasKeyJustReleased( 'T' ) && m_timeScale == .1f )
		m_timeScale = 1.f;
	//speed up feature
	if( g_theInput->IsKeyDown( 'Y' ) && m_timeScale == 1.f )
	{
		m_timeScale = 4.f;
	}
	else if( g_theInput->WasKeyJustReleased( 'Y' ) && m_timeScale == 4.f )
		m_timeScale = 1.f;
	//pause, if dead, ignore
	if(!m_isPlayerDead )
		TogglePauseState();

	//trial: spawn friendly allay
	if( g_theInput->WasKeyJustPressed( 'N' ) )
	{
		Map* curMap = m_theWorld->GetCurrentMap();
		curMap->SpawnNPC( ENTITY_TYPE_NPC_TANK, FACTION_GOOD );
		curMap->SpawnNPC( ENTITY_TYPE_NPC_TURRET, FACTION_GOOD );
	}
}

void Game::UpdateCamera(float deltaTime)
{
	float numTilesInViewVertically = static_cast<float>(m_numTilesInViewVertically);
	Vec2 halfDim( numTilesInViewVertically * CLIENT_ASPECT*.5f, numTilesInViewVertically*.5f );
	AABB2 camBounds = m_worldCamera->GetBounds();
	const Map* currentMap = m_theWorld->GetCurrentMap();
	AABB2 mapBounds( Vec2( 0.f, 0.f ), Vec2( (float)currentMap->m_size.x, (float)currentMap->m_size.y ) );
	if( g_isFullScreenMap )
	{
		camBounds.FitInBoundsAndResize( mapBounds );
		m_worldCamPos = camBounds.GetCenter();
	}
	else
	{
		Entity* player = currentMap->GetPlayerAlive();
		if( player != nullptr ) //just show the local map around the player
		{
			//camBounds.SetDimensions( Vec2( numTilesInViewVertically * CLIENT_ASPECT, numTilesInViewVertically ) );
			camBounds.SetCenter( player->m_position );
			camBounds.FitWithinBounds( mapBounds );
			m_worldCamPos = camBounds.GetCenter();
		}
	}
	if( m_cameraShakeFraction>0.f )
	{
		float cameraShiftX = m_RNG->RollRandomFloatInRange( -CAMERA_SHAKE_RANGE * m_cameraShakeFraction, CAMERA_SHAKE_RANGE * m_cameraShakeFraction );
		float cameraShiftY = m_RNG->RollRandomFloatInRange( -CAMERA_SHAKE_RANGE * m_cameraShakeFraction, CAMERA_SHAKE_RANGE * m_cameraShakeFraction );
		camBounds.Translate( Vec2( cameraShiftX * cameraShiftX, cameraShiftY * cameraShiftY ) );
		m_cameraShakeFraction -= deltaTime / CAMERA_SHAKE_RANGE;
	}
	m_worldCamera->SetOrthoView(camBounds.mins,camBounds.maxs);
	m_worldCamera->SetProjectionOrthographic(camBounds.GetDimensions().y);
}

void Game::UpdateForTitle()
{
	m_isPlayerDead = false;
	m_sceneCountdown = 0.f;
	m_alphaCountup = 0.f;
	g_theInput->SetVibrationValue( 0, 0, 0 );
	if( g_theInput->WasKeyJustPressed( KEY_SPACEBAR ) )
	{
		m_gameState = GAME_STATE_PLAYING;
		m_theWorld->StartLevel();
	}
		
	const XboxController& controller = g_theInput->GetXboxController( 0 );
	if( controller.IsConnected() && controller.GetButtonState( XBOX_BUTTON_ID_START ).WasJustPressed() )
	{
		m_gameState = GAME_STATE_PLAYING;
		m_theWorld->StartLevel();
	}
}

void Game::UpdateForWin()
{	
	if( g_theInput->WasKeyJustPressed( 'P' ) || g_theInput->WasKeyJustPressed( KEY_ESC ) )
	{
		m_gameState = GAME_STATE_TITLE;
	}
	const XboxController& controller = g_theInput->GetXboxController( 0 );
	if( controller.IsConnected() )
	{
		if( controller.GetButtonState( XBOX_BUTTON_ID_BACK ).WasJustPressed()
			|| controller.GetButtonState( XBOX_BUTTON_ID_START ).WasJustPressed() )
			m_gameState = GAME_STATE_TITLE;
	}
}

void Game::UpdateForPlayerDeath(float deltaSeconds)
{
	if( m_sceneCountdown <= 0.f )
	{		
		if( !m_isPlayerDead )
		{
			Map* curMap = m_theWorld->GetCurrentMap();
			if( curMap == nullptr )
				return;
			Entity* player = curMap->GetPlayerAlive();
			if( player == nullptr )
			{
				m_sceneCountdown = PLAYER_DEATH_TRANSITION;				
				m_alphaCountup = 0.f;
				m_isPlayerDead = true;
				m_cameraShakeFraction = 1.f;
			}
		}
		else//fade finished
		{
			g_theInput->SetVibrationValue( 0, 0.f, 0.f );
			if( m_playerRespawnChances <= 0 )//already use all chances, lose
			{
				m_gameState = GAME_STATE_LOSE;
				m_sceneCountdown = PLAYER_DEATH_TRANSITION;
				return;
			}
			//respawn player
			const XboxController controller = g_theInput->GetXboxController( 0 );
			if( g_theInput->WasKeyJustPressed( 'P' ) || 
				(controller.IsConnected() && controller.GetButtonState( XBOX_BUTTON_ID_START ).WasJustPressed()))
			{
				m_theWorld->GetCurrentMap()->SpawnPlayer( FACTION_GOOD, Vec2( 1.5f, 1.5f ) );
				m_alphaCountup = 0.f;
				m_sceneCountdown = QUICK_SCENE_TRANSITION;
				m_isPlayerDead = false;
			}
			//quit to title
			if( g_theInput->WasKeyJustPressed( KEY_ESC ) ||
				(controller.IsConnected() && controller.GetButtonState( XBOX_BUTTON_ID_BACK ).WasJustPressed()) )
			{
				m_gameState = GAME_STATE_TITLE;
			}
		}
	}
	else
	{
		g_theInput->SetVibrationValue( 0, .7f, .7f );
		m_alphaCountup += deltaSeconds;
		m_sceneCountdown -= deltaSeconds;
	}
}

void Game::RenderUITitle() const
{
	std::vector<Vertex_PCU> textVerts;
	switch( m_gameState )
	{
		case GAME_STATE_WIN:
		{
			AppendVertsForTexts( textVerts, Stringf("You Win!"), Vec2( .5f, .5f ), 1.f, Rgba8( 255, 255, 0 ) );
			if(m_sceneCountdown<=0.f )
				AppendVertsForTexts( textVerts, Stringf( "Press Start/Back/P/ESC to title" ), Vec2( .5f, .3f ), .3f, Rgba8::WHITE );
			break;
		}
		case GAME_STATE_LOSE: 
		{
			AppendVertsForTexts( textVerts, Stringf( "You Lose!" ), Vec2( .5f, .5f ), 1.f, Rgba8( 255, 0, 0 ) );
			break;
		}
		case GAME_STATE_PAUSE:
		{
			AppendVertsForTexts( textVerts, Stringf( "Pause" ), Vec2( .5f, .5f ), .75f, Rgba8::WHITE );
			AppendVertsForTexts( textVerts, Stringf( "Press Start/P to resume\nPress Back/ESC to quit" ), Vec2( .5f, .3f ), .3f, Rgba8::WHITE );
			break;
		}
		case GAME_STATE_PLAYING:
		{
			if( m_isPlayerDead && m_sceneCountdown <= 0.f )
			{
				AppendVertsForTexts( textVerts, Stringf( "You Die!" ), Vec2( .5f, .5f ), 1.f, Rgba8( 255, 0, 0 ) );
				AppendVertsForTexts( textVerts, Stringf( "Press Start/P to resume\nPress Back/ESC to quit" ), Vec2( .5f, .3f ), .3f, Rgba8::WHITE );
			}
			break;
		}
		case GAME_STATE_TITLE:
		{
			AppendVertsForTexts( textVerts, Stringf( "Incursion" ), Vec2( .5f, .5f ), 1.f, Rgba8::WHITE );
			AppendVertsForTexts( textVerts, Stringf( "Press Start/Spacebar to start" ), Vec2( .5f, .3f ), .3f, Rgba8::WHITE );
			break;
		}
		case GAME_STATE_LOADING:
		{
			AppendVertsForTexts( textVerts, Stringf( "Incursion" ), Vec2( .5f, .5f ), 1.f, Rgba8::WHITE );
			AppendVertsForTexts( textVerts, Stringf( "Loading..." ), Vec2( .5f, .3f ), .75f, Rgba8::WHITE );
			break;
		}
	}
	if( textVerts.size() > 0 )
	{		
		g_theRenderer->BindDiffuseTexture( g_theFont->GetTexture() );
		g_theRenderer->DrawVertexArray( textVerts );
	}
}

void Game::AppendVertsForTexts( std::vector<Vertex_PCU>& verts, std::string text, const Vec2& relativeCenterPos, float size, const Rgba8& tint ) const
{
	AABB2 bound = m_uiCamera->GetBounds();
	Vec2 bottomLeftUI = bound.mins;
	Vec2 upperRightUI = bound.maxs;
	Vec2 startPos;
	float width = 0.f;
	for( int cID = 0; cID < (int)text.size(); cID++ )
	{
		if( text[cID] != '\n' )
			width += size;
		else break;
	}
	float halfWidth = width*.5f;
	startPos.x = RangeMapFloat( .0f, 1.f, bottomLeftUI.x, upperRightUI.x, relativeCenterPos.x )-halfWidth;
	startPos.y = RangeMapFloat( .0f, 1.f, bottomLeftUI.y, upperRightUI.y, relativeCenterPos.y )-size*.5f;
	g_theFont->AddVertsForText2D( verts, startPos, size, text, tint );
}

void Game::RenderUIForPlay() const
{
	AABB2 bound = m_uiCamera->GetBounds();
	Vec2 bottomLeftUI = bound.mins;
	Vec2 upperRightUI = bound.maxs;
	//render player icon
	Vec2 currentIconPosition = Vec2( bottomLeftUI.x + 2 * ICON_INTERVAL, upperRightUI.y - 2 * ICON_INTERVAL );
	float iconScale = ICON_INTERVAL / PLAYER_COSMETIC_RADIUS;
	for( int playerIconIndex = 0; playerIconIndex < m_playerRespawnChances; playerIconIndex++ )
	{
		RenderPlayerIconUI( currentIconPosition, iconScale );
		currentIconPosition.x += 3 * ICON_INTERVAL;
	}
	//render faction bomb icon
	Entity* player = m_theWorld->GetCurrentMap()->GetPlayerAlive();
	if( player != nullptr && player->m_factionBombNum > 0 )
	{
		for( int bID = 0; bID < player->m_factionBombNum; bID++ )
		{
			RenderBombIconUI( currentIconPosition, iconScale );
			currentIconPosition.x += 3 * ICON_INTERVAL;
		}
	}
	//overlay screens
	if( m_gameState == GAME_STATE_PAUSE || (m_gameState==GAME_STATE_WIN&& m_sceneCountdown <= 0.f) )
	{
		g_theRenderer->BindDiffuseTexture( (Texture*)nullptr );
		g_theRenderer->DrawAABB2D( AABB2( bottomLeftUI, upperRightUI ), Rgba8( 0, 0, 0, OVERLAY_ALPHA ) );
	}
	else if( m_isPlayerDead )
	{
		g_theRenderer->BindDiffuseTexture((Texture*)nullptr );
		unsigned char newAlpha = static_cast<unsigned char>(m_alphaCountup / PLAYER_DEATH_TRANSITION * (float)OVERLAY_ALPHA);
		g_theRenderer->DrawAABB2D( AABB2( bottomLeftUI, upperRightUI ), Rgba8( 0, 0, 0, newAlpha ) );
	}
	else if( m_sceneCountdown>0.f )
	{
		g_theRenderer->BindDiffuseTexture((Texture*)nullptr );
		unsigned char newAlpha = static_cast<unsigned char>(m_alphaCountup /QUICK_SCENE_TRANSITION * (float)OVERLAY_ALPHA);
		if(m_gameState==GAME_STATE_PLAYING )
			g_theRenderer->DrawAABB2D( AABB2( bottomLeftUI, upperRightUI ), Rgba8( 0, 0, 0, (unsigned char)OVERLAY_ALPHA - newAlpha ) );
		else if(m_gameState==GAME_STATE_WIN )
			g_theRenderer->DrawAABB2D( AABB2( bottomLeftUI, upperRightUI ), Rgba8( 0, 0, 0, newAlpha ) );
	}
}

void Game::RenderPlayerIconUI(const Vec2& position, float scale) const
{
	std::vector<Vertex_PCU> iconVerts;
	AppendVertsForAABB2D(iconVerts,AABB2(-.5f,-.5f, .5f,.5f));
	TransformVertexArray( 6, &iconVerts[0], scale, 0.f, position, Rgba8(255,255,255,200) );
	Texture* baseTank = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/PlayerTankBase.png" );
	g_theRenderer->BindDiffuseTexture( baseTank );
	g_theRenderer->DrawVertexArray( iconVerts );

	Texture* turretTank = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/PlayerTankTop.png" );
	g_theRenderer->BindDiffuseTexture( turretTank );
	g_theRenderer->DrawVertexArray( iconVerts );
}

void Game::RenderBombIconUI( const Vec2& position, float scale ) const
{
	Texture* pickupTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Extras_4x4.png" );
	const SpriteSheet* sheet = new SpriteSheet( *pickupTexture, IntVec2( 4, 4 ) );
	Vec2 uvAtMins, uvAtMaxs;
	sheet->GetSpriteUVs( uvAtMins, uvAtMaxs, 13 );
	std::vector<Vertex_PCU> iconVerts;
	AppendVertsForAABB2D( iconVerts, AABB2( -.5f, -.5f, .5f, .5f ), uvAtMins, uvAtMaxs, Rgba8( 0, 0, 255 ) );
	TransformVertexArray( 6, &iconVerts[0], scale, 0.f, position, Rgba8( 255, 255, 255, 200 ) );
	
	g_theRenderer->BindDiffuseTexture( pickupTexture );
	g_theRenderer->DrawVertexArray( iconVerts );
}
