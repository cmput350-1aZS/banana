#pragma once

#include "Scene.h"

namespace ECSEngine {

template <typename... Components>
class KeyboardSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("KeyboardSystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine, SceneView<Components...>* scene) override {
        auto& em = scene->GetEntityManager();
        auto& keys = engine->GetKeys();

        for (auto& e : em) {
            auto id = e.GetID();
            if (em.IsValid(id)) {
                if (em.template HasComponent<InputComponent>(id)) {
                    auto& inputComponent = em.template GetComponent<InputComponent>(id);
                    inputComponent.keydown = keys;
                }
            }
        }

        return kContinue;
    }
};

}  // namespace ECSEngine
