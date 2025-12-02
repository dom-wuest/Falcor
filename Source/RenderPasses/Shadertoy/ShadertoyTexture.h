#pragma once
#include <cstdint>
#include <optional>
#include <string>

#include "Falcor.h"

using namespace Falcor;

enum class Filter : uint8_t { Nearest = 0, Linear = 1, Mipmap = 2 };
enum class Wrap : uint8_t { Repeat = 0, Clamp = 1 };
enum class Shape : uint8_t { Texture2D = 0, TextureCube = 1 };


class ShadertoyTexture : public Object
{
public:
    ShadertoyTexture(ref<Device> pDevice, const std::string& texturePath, Filter filter, Wrap wrap, bool srgb);
    static ref<ShadertoyTexture> create(ref<Device> pDevice, const std::string& texturePath, Filter filter = Filter::Linear, Wrap wrap = Wrap::Repeat, bool srgb = false)
    {
        return make_ref<ShadertoyTexture>(pDevice, texturePath, filter, wrap, srgb);
    }
    ref<Texture> getTexture() const { return mpTexture; }
    ref<Sampler> getSampler() const { return mpSampler; }

    std::string getTexturePath() const { return mTexturePath; }
    Filter getFilter() const { return mFilter; }
    Wrap getWrap() const { return mWrap; }
    bool isSrgb() const { return mSrgb; }
    Shape getShape() const
    {
        if (mpTexture->getType() == Texture::Type::TextureCube)
            return Shape::TextureCube;
        else
            return Shape::Texture2D;
    }


private:
    std::string mTexturePath;
    Filter mFilter;
    Wrap mWrap;
    bool mSrgb;

    ref<Texture> mpTexture;
    ref<Sampler> mpSampler;
};
