#pragma once
#include "Engine/Core/EngineCommon.hpp"

class App;
class RenderContext;
class InputSystem;
class Game;
class AudioSystem;
class BitmapFont;

constexpr int   CAMERA_VIEW_SIZE_Y = 9;
constexpr float CLIENT_ASPECT = 16.f/9.f; // We are requesting a 2:1 aspect (square) window area

constexpr float BULLET_SPEED = 2.f;
constexpr float BULLET_PHYSICS_RADIUS = .05f;
constexpr float BULLET_COSMETIC_RADIUS = .05f;

constexpr float NPC_TURRET_PHYSICS_RADIUS = .29f;
constexpr float NPC_TURRET_COSMETIC_RADIUS = .4f;
constexpr float NPC_TURRET_SHOOT_COOLDOWN = 1.3f;
constexpr float NPC_TURRET_DETECT_LENGTH = 15.f;
constexpr float NPC_TURRET_SHOOT_DEGREES = 5.f;
constexpr float NPC_TURRET_TURN_SPEED = 100.f;
constexpr float NPC_TURRET_SCAN_RANGE = 45.f;
constexpr int   NPC_TURRET_HEALTH = 5;

constexpr float NPC_TANK_PHYSICS_RADIUS = .29f;
constexpr float NPC_TANK_COSMETIC_RADIUS = .4f;
constexpr float NPC_TANK_SPEED = .8f;
constexpr float NPC_TANK_TURN_COUNTDOWN = 2.f;
constexpr float NPC_TANK_SHOOT_COOLDOWN = 1.7f;
constexpr float NPC_TANK_FORWARD_DEGREES = 45.f;
constexpr float NPC_TANK_SHOOT_DEGREES = 5.f;
constexpr float NPC_TANK_DETECT_LENGTH = 10.f;
constexpr float NPC_TANK_TURN_SPEED = 100.f;
constexpr int   NPC_TANK_NUM = 10;
constexpr int   NPC_TANK_HEALTH = 3;

constexpr float PLAYER_SPEED = 1.f;
constexpr float PLAYER_TURN_SPEED = 180.f;
constexpr float PLAYER_GUN_TURN_SPEED = 270.f;
constexpr float PLAYER_PHYSICS_RADIUS = .29f;
constexpr float PLAYER_COSMETIC_RADIUS = .4f;
constexpr int   PLAYER_HEALTH = 8;
constexpr int   PLAYER_RESPAWN_TIMES = 4;
constexpr float PLAYER_RESPAWN_INTERVAL = 3.f;
constexpr float PLAYER_HIT_VIBRATION_TIME = .2f;

constexpr float BOULDER_RADIUS = .29f;

constexpr float BOMB_EXPLOSION_TIME = 1.f;
constexpr float BOMB_EXPLOSION_RADIUS = 4.f;

constexpr float PICKUP_RADIUS = .3f;

constexpr float EXPLOSION_MAX_DURATION = 3.f;

constexpr int   PICKUP_HEALTH_VALUE = 1;

constexpr float CAMERA_SHAKE_RANGE = 1.f;
constexpr float PLAYER_DEATH_TRANSITION = 2.f;
constexpr float QUICK_SCENE_TRANSITION = .5f;
constexpr int   OVERLAY_ALPHA = 100;
constexpr float ICON_INTERVAL = .2f;
constexpr float LINE_THICKNESS = .03f;
constexpr int   RAYCAST_SAMPLE_RATE = 50;
constexpr float HEALTH_BAR_LENGTH = 1.f;

extern App* g_theApp;
extern RenderContext* g_theRenderer;
extern InputSystem* g_theInput;
extern AudioSystem* g_theAudio;
extern Game* g_theGame;
extern BitmapFont* g_theFont;

extern bool g_isDebugDrawing;
extern bool g_isFullScreenMap;
extern bool g_isPhysicsEnabled;
