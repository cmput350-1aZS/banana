#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

#include "MathUtil.h"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace ECSEngine {

using SpriteID = size_t;
class SpriteManager {
public:
    SpriteManager();
    ~SpriteManager() = default;
    // Copies the sourceRect part of the PNG at texturePath into the shared atlas and returns the ID
    // of a sprite that shows it. Throws if the file can't be loaded or the atlas is full.
    [[nodiscard]] SpriteID RegisterTexture(const std::string& texturePath, const Rect& sourceRect);
    // Throws std::out_of_range if id was never returned by RegisterTexture.
    sf::Sprite& GetSprite(SpriteID id);
    // The atlas every sprite is drawn from (one texture means one draw call later).
    sf::Texture& GetTexture() { return mTexture; }

private:
    // Row packing: where the next sprite goes in the atlas, all in atlas pixels.
    //   mColMinX: x of the next free spot on the current row.
    //   mRowMinY: y of the top of the current row.
    //   mRowMaxY: y of the bottom of the tallest sprite on the current row, which is where the
    //             next row starts.
    int mRowMinY, mRowMaxY, mColMinX;
    // The shared atlas. Every sprite keeps a pointer to it, so a SpriteManager must not be copied
    // or moved after sprites exist.
    sf::Texture mTexture;
    std::unordered_map<std::string, SpriteID> mSpriteMap;
    // A SpriteID is an index into this vector.
    std::vector<sf::Sprite> mSprites;
};

}  // namespace ECSEngine
