#pragma once

#include <bitset>
#include <string>
#include <unordered_map>

#include "CameraComponent.h"
#include "Entity.h"
#include "EntityManager.h"
#include "LocationComponent.h"
#include "MathUtil.h"
#include "Scene.h"
#include "SpriteComponent.h"
#include "SpriteManager.h"
#include "SystemManager.h"
#include "WindowManager.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/VertexArray.hpp>

namespace ECSEngine {

template <typename... Components>
class SpriteSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static const std::string name("SpriteSystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine, SceneView<Components...>* scene) override {
        auto& entities = scene->GetEntityManager();
        auto& spriteManager = engine->GetSpriteManager();
        auto& windowManager = scene->GetWindowManager();

        auto window = windowManager.GetWindow();

        if (!window || !window->isOpen()) {
            return kContinue;
        }

        window->clear();

        sf::VertexArray vertices(sf::PrimitiveType::Triangles);  // an empty list of corners

        for (const Entity& entity : entities) {
            EntityID id = entity.GetID();

            if (!entities.IsValid(id) || !entities.template HasComponent<SpriteComponent>(id)) {
                continue;
            }

            const auto& spriteComponent = entities.template GetComponent<SpriteComponent>(id);

            Point2D basePosition(0.0f, 0.0f);

            if (entities.template HasComponent<LocationComponent>(id)) {
                basePosition = entities.template GetComponent<LocationComponent>(id).loc;
            }

            Point2D drawPosition(basePosition.x + spriteComponent.bounds.topLeft.x,
                                 basePosition.y + spriteComponent.bounds.topLeft.y);

            if (spriteComponent.worldSpace) {
                drawPosition = windowManager.WorldToWindow(drawPosition);
            }

            sf::Sprite sprite = spriteManager.GetSprite(spriteComponent.textureID);

            sf::IntRect sourceRect = sprite.getTextureRect();

            if (sourceRect.size.x <= 0 || sourceRect.size.y <= 0) {
                continue;
            }

            float scaleX = spriteComponent.bounds.width / static_cast<float>(sourceRect.size.x);

            float scaleY = spriteComponent.bounds.height / static_cast<float>(sourceRect.size.y);

            if (spriteComponent.worldSpace) {
                const float worldScale = windowManager.GetWorldScale();

                if (worldScale <= 0.0f) {
                    continue;
                }

                scaleX /= worldScale;
                scaleY /= worldScale;
            }

            float screenRight = drawPosition.x + static_cast<float>(sourceRect.size.x) * scaleX;
            float screenLeft = drawPosition.x;
            float screenTop = drawPosition.y;
            float screenBottom = drawPosition.y + static_cast<float>(sourceRect.size.y) * scaleY;

            float sheetLeft = sourceRect.position.x;
            float sheetRight = sourceRect.position.x + sourceRect.size.x;
            float sheetTop = sourceRect.position.y;
            float sheetBottom = sourceRect.position.y + sourceRect.size.y;

            sf::Vertex topLeft;
            topLeft.position = {screenLeft, screenTop};
            topLeft.texCoords = {sheetLeft, sheetTop};
            vertices.append(topLeft);

            sf::Vertex topRight;
            topRight.position = {screenRight, screenTop};
            topRight.texCoords = {sheetRight, sheetTop};
            vertices.append(topRight);

            sf::Vertex bottomLeft;
            bottomLeft.position = {screenLeft, screenBottom};
            bottomLeft.texCoords = {sheetLeft, sheetBottom};
            vertices.append(bottomLeft);

            sf::Vertex bottomRight;
            bottomRight.position = {screenRight, screenBottom};
            bottomRight.texCoords = {sheetRight, sheetBottom};
            vertices.append(bottomRight);

            vertices.append(topRight);
            vertices.append(bottomLeft);


        }

        window->draw(vertices, &spriteManager.GetTexture());

        return kContinue;
    }
};

}  // namespace ECSEngine