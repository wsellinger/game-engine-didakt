#include "Engine.h"

#include "Config.h"
#include "Logger.h"

#include "../providers/ProviderManagerFactory.h"

#include <string>

using string = std::string;

//=== Public ===

bool Engine::Initialize()
{
    //Get Providers
    _providerManager = CreateProviderManager();

    //Init Logging
    Logger::SetProvider(_providerManager->GetLoggingProvider());

    //Load Config
    _config = LoadConfig(Config::DEFAULT_PATH);

    //Init Providers
    bool providerInitialized = _providerManager->Initialize(_config);
    if (!providerInitialized)
        return false;

    //Init Managers
    _assetManager.Initialize(_providerManager->GetAssetProvider());
    _inputManager.Initialize(_providerManager->GetInputProvider());

    _isRunning = true;

    return true;
}

void Engine::Run()
{
    //Init Time
    ISystemTimerProvider& timerProvider = _providerManager->GetSystemTimerProvider();

    uint64_t  initialTime = timerProvider.GetTime();
    uint64_t  frequency = timerProvider.GetFrequency();

    TimeTracker timeTracker(initialTime, frequency);
    FixedTimer fixedTimer;
    FPSMeter fpsMeter(initialTime, frequency);

    //Main Loop
    while (_isRunning)
    {
        //Update Time
        uint64_t  currentTime = timerProvider.GetTime();
        double deltaTime = timeTracker.GetDeltaTime(currentTime);
        
        //Events
        ProcessEvents();

        //Input
        _inputManager.Update();

        //Fixed Update
        fixedTimer.Increment(deltaTime);
        while (fixedTimer.Update())
        {
            FixedUpdate(fixedTimer.GetTimestep());
        }

        FrameUpdate(deltaTime);

        //Render
        Render();

        //FPS Update
        if (fpsMeter.Update(currentTime))
        {
            string title = _config.window.title + " | FPS: " + std::to_string(fpsMeter.GetFPS());
            _providerManager->GetWindowProvider().SetTitle(title);
        }
    }
}

void Engine::Shutdown()
{
    _assetManager.ClearAll();

    //Shutdown Providers
    _providerManager->Shutdown();
}

//=== Private ===

void Engine::ProcessEvents()
{
    ISystemEventProvider& eventProvider = _providerManager->GetSystemEventProvider();
    eventProvider.PollEvents();

    if (eventProvider.QuitRequested())
    {
        _isRunning = false;
    }
}

void Engine::FixedUpdate(double deltaTime)
{
    _movementSystem.Update(_sceneManager.GetCurrentScene().GetRegistry(), deltaTime);
}

void Engine::FrameUpdate(double deltaTime)
{

}

void Engine::Render()
{
    //Render Background
    RenderColor& color = _config.renderer.clearColor;
    IRenderProvider& renderProvider = _providerManager->GetRenderProvider();

    renderProvider.Clear(color);
    _renderSystem.Render(_sceneManager.GetCurrentScene().GetRegistry(), renderProvider, _assetManager, _camera);
    renderProvider.Present();
}


//=== TimeTracker ===

double TimeTracker::GetDeltaTime(uint64_t  currentTime)
{
    double deltaTime = static_cast<double>(currentTime - _time) / _frequency;
    _time = currentTime;

    //Prevent Spiral
    if (deltaTime > MAX_DELTA_TIME)
        deltaTime = MAX_DELTA_TIME;

    return deltaTime;
}

//=== LogicTimer ===

bool FixedTimer::Update()
{
    if (_accumulator >= FIXED_TIMESTEP)
    {
        _accumulator -= FIXED_TIMESTEP;
        return true;
    }

    return false;
}

//=== FPSMeter ===

bool FPSMeter::Update(uint64_t  currentTime)
{
    _frames++;
    double time = static_cast<double>(currentTime - _time) / _frequency;

    if (time >= UPDATE_TIME)
    {
        _fps = static_cast<int>(_frames / time);
        _frames = 0;
        _time = currentTime;

        return true;
    }

    return false;
}
