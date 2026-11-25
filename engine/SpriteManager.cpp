#include "SpriteManager.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include <SFML/Graphics/Image.hpp>

namespace ECSEngine {

SpriteManager::SpriteManager() {
    // Empty atlas: the first sprite goes in the top-left corner.
    mRowMinY = 0;
    mRowMaxY = 0;
    mColMinX = 0;

    if (!mTexture.resize({2048, 2048})) {
        throw std::runtime_error("Could not create the sprite atlas");
    }
}

SpriteID SpriteManager::RegisterTexture(const std::string& texturePath, const Rect& sourceRect) {
    // Whole pixels as ints: mColMinX and mRowMinY are ints, and SFML's unsigned sizes would warn
    // when compared with them.
    int width = static_cast<int>(sourceRect.width);
    int height = static_cast<int>(sourceRect.height);

    // Doesn't fit on the rest of this row: start a new row under the tallest sprite of the old one.
    if (mColMinX + width > static_cast<int>(mTexture.getSize().x)) {
        mRowMinY = mRowMaxY;
        mColMinX = 0;
    }

    // Checked after the row move, since that decides where the sprite will sit. If it still runs
    // off the bottom, there is no room left in the atlas.
    if (mRowMinY + height > static_cast<int>(mTexture.getSize().y)) {
        throw std::runtime_error("too large of height");
    }

    // source: the whole PNG in RAM. area: the block of it that holds this sprite.
    sf::Image source(texturePath);
    sf::IntRect area(
        {static_cast<int>(sourceRect.topLeft.x), static_cast<int>(sourceRect.topLeft.y)},
        {width, height});

    // A Texture can't copy out part of an Image, so cut the sprite into its own Image first.
    // {0, 0} is the corner of piece.
    sf::Image piece({static_cast<unsigned>(width), static_cast<unsigned>(height)});
    if (!piece.copy(source, {0, 0}, area)) {
        throw std::runtime_error("failed to copy");
    }

    // Paste the sprite into the atlas at the next free spot, (mColMinX, mRowMinY).
    mTexture.update(piece, {static_cast<unsigned>(mColMinX), static_cast<unsigned>(mRowMinY)});

    // Move the next free spot to the right of this sprite, and lower the row's bottom if this
    // sprite is the tallest on the row so far.
    mColMinX += width;
    mRowMaxY = std::max(mRowMaxY, mRowMinY + height);

    // Where the sprite now sits in the atlas. mColMinX has already moved past it, so step back by
    // width to get its left edge.
    sf::IntRect placed({mColMinX - width, mRowMinY}, {width, height});

    mSprites.emplace_back(mTexture, placed);

    return mSprites.size() - 1;
}

sf::Sprite& SpriteManager::GetSprite(SpriteID id) {
    // static sf::Texture dummyTexture;
    // static sf::Sprite dummy(dummyTexture);
    // return dummy;
    if (id >= mSprites.size()) {
        throw std::out_of_range("Invalid sprite ID");
    }
    return mSprites[id];
}

}  // namespace ECSEngine
