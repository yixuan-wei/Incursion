#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Camera.hpp"
#include <string>
#include <vector>

class RandomNumberGenerator;
class Entity;
struct Vec2;
class World;
class Clock;
struct Vertex_PCU;
struct Rgba8;

enum GameState
{
	GAME_STATE_LOADING=0,
	GAME_STATE_TITLE,
	GAME_STATE_PLAYING,
	GAME_STATE_WIN,
	GAME_STATE_PAUSE,	
	GAME_STATE_LOSE,

	NUM_GAME_STATES
};

class Game {
public:
	Game() {}
	~Game() {}
	void Startup();
	void Shutdown();
	void Update();
	void Render()const;

	bool IsInPlayState()const;
	const GameState& GetCurrentGameState()const { return m_gameState; }
	void ProgressToState(GameState nextState);

	const int m_numTilesInViewVertically = CAMERA_VIEW_SIZE_Y;
	Camera* m_worldCamera = nullptr;
	int m_playerRespawnChances = PLAYER_RESPAWN_TIMES;
	RandomNumberGenerator* m_RNG = nullptr;

private:
	Clock* m_gameClock = nullptr;
	World* m_theWorld = nullptr;
	float m_timeScale = 1.f;
	float m_sceneCountdown = 0.f;
	float m_alphaCountup = 0.f;
	float m_cameraShakeFraction = 0.f;
	bool m_isPlayerDead = false;
	bool m_hasPlayerWon = false;
	Camera* m_uiCamera = nullptr;
	Vec2 m_worldCamPos;
	GameState m_gameState = GAME_STATE_LOADING;
	bool m_loadingStarted = false;
	GameState m_lastGameState = m_gameState;

	void LoadAssets();
	void TogglePauseState();
	void SetPauseState();
	
	void UpdateEventStates();
	void UpdateCamera( float deltaSeconds);
	void UpdateForTitle();
	void UpdateForWin();
	void UpdateForPlayerDeath(float deltaSeconds);

	void RenderUITitle() const;
	void AppendVertsForTexts(std::vector<Vertex_PCU>& verts,std::string text, const Vec2& relativeCenterPos, float size, const Rgba8& tint) const;
	void RenderUIForPlay()const;
	void RenderPlayerIconUI(const Vec2& position, float scale)const;
	void RenderBombIconUI(const Vec2& position, float scale )const;
};