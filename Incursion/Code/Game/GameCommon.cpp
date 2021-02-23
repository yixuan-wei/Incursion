#include "Game/GameCommon.hpp"

App* g_theApp = nullptr;
RenderContext* g_theRenderer = nullptr;
InputSystem* g_theInput = nullptr;
Game* g_theGame = nullptr;
AudioSystem* g_theAudio = nullptr;
BitmapFont* g_theFont = nullptr;

bool g_isDebugDrawing = false;
bool g_isFullScreenMap = false;
bool g_isPhysicsEnabled = false;
