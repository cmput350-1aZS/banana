#pragma once

#include <algorithm>
#include <memory>
#include <vector>

#include "CameraComponent.h"
#include "CollisionComponent.h"
#include "EntityManager.h"
#include "InputComponent.h"
#include "LocationComponent.h"
#include "MathUtil.h"
#include "Scene.h"
#include "ScoreComponent.h"
#include "SoundManager.h"
#include "SpawnComponent.h"
#include "SpriteComponent.h"
#include "SpriteManager.h"
#include "SystemManager.h"
#include "WindowManager.h"
#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window.hpp>

namespace ECSEngine {

/**
 * @brief What a system is allowed to see of the engine: the shared managers, the frame time and
 * the scene stack.
 *
 * Systems receive an EngineView*, not the ECSEngine itself, so they can use the shared resources
 * and start or leave scenes, but cannot run the engine loop. A scene's own entities come from
 * the SceneView a system is given, not from here.
 *
 * @tparam Components The component types the game uses.
 */
template <typename... Components>
class EngineView {
public:
    virtual SoundManager& GetSoundManager() = 0;
    virtual SpriteManager& GetSpriteManager() = 0;
    virtual WindowManager& GetWindowManager() = 0;

    /**
     * @brief Gives the duration of the last frame.
     *
     * Pure virtual (= 0): EngineView has no clock, so ECSEngine supplies the value.
     *
     * @return Seconds the previous frame took. It is 0 until the first frame has been timed.
     */
    virtual float GetFrameTime() = 0;

    virtual const std::bitset<sf::Keyboard::ScancodeCount>& GetKeys() = 0;

    /**
     * @brief Tells whether debug drawing is switched on (the D key toggles it).
     *
     *
     * @return True while collision boxes should be drawn.
     */
    virtual bool GetDebug() = 0;

    /**
     * @brief Gives the top scene, the one the engine runs this frame.
     *
     * @return The last scene on the stack. The stack must not be empty.
     */
    virtual std::shared_ptr<SceneView<Components...>> GetScene() = 0;

    /**
     * @brief Creates a new, empty scene that shares the engine's window.
     *
     * Only the engine can build a scene (the ECSScene constructor is private), so systems ask
     * for one here. The scene is not on the stack until it is passed to AddScene.
     *
     * 
    */
    virtual std::shared_ptr<SceneView<Components...>> MakeScene() = 0;

    /**
     * @brief Asks for a scene to be pushed on the stack.
     *
     * The scene waits in a list and goes on the stack at the end of the current frame (or at the
     * start of Run() if it was added before that), after any finished scene has been popped.
     *
     * @param scene The scene to add, usually one made by MakeScene.
     */
    virtual void AddScene(std::shared_ptr<SceneView<Components...>> scene) = 0;

    /**
     * @brief Removes the top scene from the stack right away.
     *
     * Run() already pops a scene that returns kSceneDone, so a system should not also call this
     * for the same scene.
     */
    virtual void PopScene() = 0;
};

/**
 * @brief Owns the shared managers and the stack of scenes, and runs the game loop.
 *
 * Each frame it handles window and keyboard events, measures the frame time, then asks the top
 * scene to run one frame. A scene that reports it is finished is popped off the stack.
 *
 * @tparam Components The component types the game uses.
 */
template <typename... Components>
class ECSEngine : public EngineView<Components...> {
public:
    /**
     * @brief Creates the engine and its window.
     *
     * @param width The window width in pixels.
     * @param height The window height in pixels.
     * @param name The window title.
     */
    ECSEngine(unsigned int width, unsigned int height, const std::string& name);
    ~ECSEngine();

    /**
     * @brief Runs the game loop until the window is closed.
     *
     * Scenes added with AddScene before this call are put on the stack first, so the first frame
     * always has a top scene to run.
     */
    void Run();

    SoundManager& GetSoundManager() override { return mSoundManager; }

    SpriteManager& GetSpriteManager() override { return mSpriteManager; }

    WindowManager& GetWindowManager() override { return mWindowManager; }

    float GetFrameTime() override { return mFrameTime; }

    const std::bitset<sf::Keyboard::ScancodeCount>& GetKeys() override { return mKeys; }


    bool GetDebug() override { return debug; }

    std::shared_ptr<SceneView<Components...>> GetScene() override { return mScenes.back(); }

    std::shared_ptr<SceneView<Components...>> MakeScene() override {
        // new, not std::make_shared: make_shared cannot reach the private ECSScene constructor
        auto ptr = new ECSScene<Components...>(mWindowManager);
        return std::shared_ptr<ECSScene<Components...>>(ptr);
    }

    void AddScene(std::shared_ptr<SceneView<Components...>> scene) override {
        mNewScenes.push_back(scene);
    }

    void PopScene() override { mScenes.pop_back(); }

private:
    /**
     * @brief Handles window and keyboard events for this frame.
     *
     * A key press or release sets or clears that key's bit in keydown of every InputComponent.
     *
     * @return False if the window was closed, so the loop should stop; true otherwise.
     */
    bool ProcessEvents();

    // The scene stack. The last element is the top scene, the one that runs.
    std::vector<std::shared_ptr<SceneView<Components...>>> mScenes;
    // The waiting room: scenes added during a frame. Run() moves them onto mScenes after the pop.
    std::vector<std::shared_ptr<SceneView<Components...>>> mNewScenes;

    SpriteManager mSpriteManager;
    SoundManager mSoundManager;
    WindowManager mWindowManager;

    float mFrameTime = 0.0f;  // Seconds the previous frame took; set once per frame in Run()
    sf::Clock mClock;         // Restarted every frame, so its reading is the frame's duration

    // For debugging - pause and step the engine
    bool running, step;
    int debug = 0;

    std::bitset<sf::Keyboard::ScancodeCount> mKeys;
};

template <typename... Components>
ECSEngine<Components...>::ECSEngine(unsigned int width, unsigned int height,
                                    const std::string& name)
    : mWindowManager(width, height, name) {}

template <typename... Components>
ECSEngine<Components...>::~ECSEngine() {}

template <typename... Components>
void ECSEngine<Components...>::Run() {
    // Set fps to 60
    mWindowManager.GetWindow()->setFramerateLimit(60);

    // Restart once before the loop so the time spent loading is not counted in frame 1
    mClock.restart();

    // Seat the scenes added before Run() so GetScene() never sees an empty stack in frame 1
    mScenes.insert(mScenes.end(), mNewScenes.begin(), mNewScenes.end());
    mNewScenes.clear();

    while (ProcessEvents() && !mScenes.empty()) {
        // restart() returns the time since the last restart, which is the previous frame's
        // duration, and starts timing the next frame
        mFrameTime = std::min(mClock.restart().asSeconds(), 1.0f / 30.0f);

        // Run the top scene for one frame. Our own copy of the pointer keeps it alive below.
        auto scene = GetScene();
        auto result = scene->DoOneFrame(this);
        // The scene says it is finished, so take it off the stack
        if (result == kSceneDone) {
            PopScene();
        }

        // Add the scenes requested this frame only after the pop, so the pop removes the
        // finished scene and never a new one
        mScenes.insert(mScenes.end(), mNewScenes.begin(), mNewScenes.end());
        mNewScenes.clear();

        mWindowManager.GetWindow()->display();
    }
}

template <typename... Components>
bool ECSEngine<Components...>::ProcessEvents() {
    auto window = mWindowManager.GetWindow();

    // pollEvent() hands back the waiting events one at a time until none are left
    while (auto event = window->pollEvent()) {
        // The window's close button was clicked: tell Run() to stop
        if (event->template is<sf::Event::Closed>()) {
            return false;
        }

        // getIf returns a pointer if the event is a key press, otherwise nothing
        auto pressed = event->template getIf<sf::Event::KeyPressed>();
        if (pressed) {
            if (pressed->scancode == sf::Keyboard::Scan::D) {
                debug = !debug;
            }

            mKeys.set(static_cast<size_t>(pressed->scancode), true);
        }

        auto released = event->template getIf<sf::Event::KeyReleased>();
        if (released) {
            mKeys.set(static_cast<size_t>(released->scancode), false);
        }
    }
    return true;
}

}  // namespace ECSEngine
