#pragma once

#include "Scene.h"

namespace ECSEngine {

/**
 * @brief Draws a box around every collidable entity of its scene while debugging is switched on.
 *
 */
template <typename... Components>
class DebugSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("DebugSystem");
        return name;
    }

    /**
     * @brief Draws red boxes for static entities and green boxes for the rest.
     *
     * @param engine The shared engine; tells whether debug drawing is on.
     * @param scene The scene being run; its entities are drawn through its own window manager.
     * @return kContinue, so the engine keeps running.
     */
    tSystemReturn Run(EngineView<Components...>* engine, SceneView<Components...>* scene) override {
        if (!engine->GetDebug()) {
            return kContinue;
        }

        auto& em = scene->GetEntityManager();
        auto& windowManager = scene->GetWindowManager();

        for (auto& e : em) {
            auto id = e.GetID();
            if (!em.IsValid(id) || !em.template HasComponent<CollisionComponent>(id) ||
                !em.template HasComponent<LocationComponent>(id)) {
                continue;
            }

            auto& collision = em.template GetComponent<CollisionComponent>(id);
            auto& location = em.template GetComponent<LocationComponent>(id);
            Rect box{Point2D(collision.collisionBox.topLeft.x + location.loc.x,
                             collision.collisionBox.topLeft.y + location.loc.y),
                     collision.collisionBox.width, collision.collisionBox.height};

            if (collision.staticEntity) {
                windowManager.DebugFrameRect(box, 255, 0, 0);
            } else {
                windowManager.DebugFrameRect(box, 0, 255, 0);
            }
        }
        return kContinue;
    }
};

}  // namespace ECSEngine
