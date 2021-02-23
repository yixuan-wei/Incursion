#include "Game/App.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Platform/Window.hpp"

App::App()
{
	
}

App::~App()
{

}

void App::Startup()
{
    //config setting
    float windowClientRatioOfHeight = g_gameConfigBlackboard->GetValue("windowHeightRatio", 0.8f);
    float aspectRatio = g_gameConfigBlackboard->GetValue("windowAspect", 16.0f / 9.0f);
    std::string windowTitle = g_gameConfigBlackboard->GetValue("windowTitle", "SD2.A01");

    g_theApp = &(*this);						//initialize global App pointer
	g_theEvents = new EventSystem();
    g_theRenderer = new RenderContext();		//initialize global RendererContext pointer
    g_theInput = new InputSystem();
	g_theConsole = new DevConsole(g_theInput);
	g_theAudio = new AudioSystem();
    g_theGame = new Game();

      //set up window
    m_theWindow = new Window();
    m_theWindow->Open(windowTitle, aspectRatio, windowClientRatioOfHeight);
    m_theWindow->SetInputSystem(g_theInput);
	
	g_theConsole->Startup();
	g_theInput->Startup();
	g_theRenderer->Startup(m_theWindow);
	g_theAudio->Startup();

	Clock::SystemStartup();

	g_theGame->Startup();

	g_isDebugDrawing = false;
	g_isFullScreenMap = false;
	g_isPhysicsEnabled = true;
}

void App::Shutdown()
{
	Clock::SystemShutdown();

	g_theGame->Shutdown();
	g_theAudio->Shutdown();
	g_theRenderer->Shutdown();
	g_theInput->Shutdown();
	g_theConsole->Shutdown();
	m_theWindow->Close();

	delete m_theWindow;
	m_theWindow = nullptr;

	delete g_theGame;
	g_theGame = nullptr;

	delete g_theAudio;
	g_theAudio = nullptr;

	delete g_theRenderer;
	g_theRenderer = nullptr;

	delete g_theInput;
	g_theInput = nullptr;

	delete g_theConsole;
	g_theConsole = nullptr;

	delete g_theEvents;
	g_theEvents = nullptr;
}

void App::RunFrame()
{
	BeginFrame();     //engine only
	Update();//game only
	Render();         //game only
	EndFrame();	      //engine only
}

bool App::HandleQuitRequisted()
{
	m_isQuiting = true;
	return true;
}

//////////////////////////////////////////////////////////////////////////
Vec2 App::GetWindowDimensions() const
{
    float width = (float)m_theWindow->GetClientWidth();
    float height = (float)m_theWindow->GetClientHeight();
    return Vec2(width, height);
}

void App::BeginFrame()
{
	Clock::BeginFrame();
	
	m_theWindow->BeginFrame();
	g_theConsole->BeginFrame();
	g_theRenderer->BeginFrame();
	g_theInput->BeginFrame();
	g_theAudio->BeginFrame();
}

void App::Update()
{
    if (m_theWindow->IsQuiting())
    {
        HandleQuitRequisted();
        return;
    }
	UpdateKeyboardStates();

	g_theGame->Update( );
	g_theConsole->Update();
}

void App::Render() const
{	
	g_theGame->Render();
}

void App::EndFrame()
{
	g_theInput->EndFrame();
	g_theRenderer->EndFrame();
	g_theAudio->EndFrame();
	g_theConsole->EndFrame();
	m_theWindow->EndFrame();
}

void App::UpdateKeyboardStates()
{
	//ESC quit
	if( g_theInput->IsKeyDown( KEY_ESC ) )
	{
		HandleQuitRequisted();
		return;
	}
}
