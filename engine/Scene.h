#pragma once

#include "EntityManager.h"
#include "SystemManager.h"
#include "WindowManager.h"

namespace ECSEngine {

template <typename... Components>
class EngineView;

/**
 * @brief What a scene reports after running one frame.
 *
 * kSceneContinue keeps the scene on the stack. kSceneDone means it is finished, and the engine
 * pops it.
 */
enum tSceneReturn { kSceneContinue, kSceneDone };

/**
 * @brief What the engine and the systems may see of one scene: its entities, its window view
 * and its systems.
 *
 * SceneView is the interface and ECSScene is the real scene, the same split as EngineView and
 * ECSEngine.
 *
 * @tparam Components The component types the game uses.
 */
template <typename... Components>
class SceneView {
public:
    virtual EntityManager<Components...>& GetEntityManager() = 0;
    virtual WindowManager& GetWindowManager() = 0;
    virtual SystemManager<Components...>& GetSystemManager() = 0;

    /**
     * @brief Runs every system of this scene once.
     *
     * @param engine The engine, for the shared managers and the frame time.
     * @return kSceneDone if a system asked to exit, otherwise kSceneContinue.
     */
    virtual tSceneReturn DoOneFrame(EngineView<Components...>* engine) = 0;
};

/**
 * @brief One screen of the game, such as the menu, the play screen or the pause screen.
 *
 * It owns its own entities, its own systems and its own window view (camera). Sprites, sounds
 * and the frame time are shared, so they come from the engine, not from here.
 *
 * @tparam Components The component types the game uses.
 */
template <typename... Components>
class ECSScene : public SceneView<Components...> {
public:
    EntityManager<Components...>& GetEntityManager() override { return mEntityManager; }
    WindowManager& GetWindowManager() override { return mWindowManager; }
    SystemManager<Components...>& GetSystemManager() override { return mSystemManager; }
    [[nodiscard]] tSceneReturn DoOneFrame(EngineView<Components...>* engine) override {
        if (!mSystemManager.Run(engine, this)) return kSceneDone;
        return kSceneContinue;
    }

private:
    // Private, with ECSEngine as a friend, so scenes can only be created by the engine's
    // MakeScene. The new scene borrows the engine's window and keeps its own camera.
    template <typename... T>
    friend class ECSEngine;
    ECSScene(WindowManager& windowManager) : mWindowManager(windowManager.GetWindow()) {}
    EntityManager<Components...> mEntityManager;
    SystemManager<Components...> mSystemManager;
    WindowManager mWindowManager;
};

}  // namespace ECSEngine