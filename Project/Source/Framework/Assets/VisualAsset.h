#pragma once

#include "Framework/Assets/Asset.h"
#include "Framework/Render/Color.h"
#include "Framework/Render/BlendMode.h"
#include "Framework/Render/Flip.h"
#include "Framework/Shapes/Rect.h"

template<class T>
class VisualAsset : public Asset<T>
{
public:

    VisualAsset() :
        Asset<T>()
    {

    }

    VisualAsset(AssetHandle<T>* handle) :
        Asset<T>(handle)
    {

    }

    VisualAsset(const VisualAsset& other) :
        Asset<T>(other)
    {
        color = other.color;
        flip = other.flip;
        mask = other.mask;
        blendMode = other.blendMode;
        angle = other.angle;
        rotationAnchor = other.rotationAnchor;
    }

    VisualAsset(VisualAsset&& other) noexcept :
        Asset<T>(std::move(other))
    {
        color = std::move(other.color);
        flip = other.flip;
        mask = std::move(other.mask);
        blendMode = other.blendMode;
        angle = other.angle;
        rotationAnchor = std::move(other.rotationAnchor);

        other.flip = Flip::NO_FLIP;
        other.blendMode = BlendMode::BLEND;
        other.angle = 0;
        other.rotationAnchor = Point::One * 0.5f;
    }

    virtual ~VisualAsset() override
    {

    }

    virtual void ResetMask()
    {
        mask = Rect::Null;
    }

    virtual void ResetColor()
    {
        color = Color::White;
    }

public: // Operators

    VisualAsset& operator=(const VisualAsset& other)
    {
        if (this == &other)
            return *this;

        Asset<T>::operator=(other);
        color = other.color;
        flip = other.flip;
        mask = other.mask;
        blendMode = other.blendMode;
        angle = other.angle;
        rotationAnchor = other.rotationAnchor;

        return *this;
    }

    VisualAsset& operator=(VisualAsset&& other) noexcept
    {
        if (this == &other)
            return *this;

        Asset<T>::operator=(std::move(other));
        color = std::move(other.color);
        flip = other.flip;
        mask = std::move(other.mask);
        blendMode = other.blendMode;
        angle = other.angle;
        rotationAnchor = std::move(other.rotationAnchor);

        other.flip = Flip::NO_FLIP;
        other.blendMode = BlendMode::BLEND;
        other.angle = 0;
        other.rotationAnchor = Point::One * 0.5f;

        return *this;
    }

public:

    // Culling visual mask
    Rect mask = Rect::Null;
    // Color mod & Transparency
    Color color = Color::White;
    // Set how the texture will be flipped
    Flip flip = Flip::NO_FLIP;
    // How color blends into the texture
    BlendMode blendMode = BlendMode::BLEND;
    // Angle of rotation
    float angle = 0;
    // Rotation Texture Anchor. From 0 to 1.
    Point rotationAnchor = Point::One * 0.5f;

};
