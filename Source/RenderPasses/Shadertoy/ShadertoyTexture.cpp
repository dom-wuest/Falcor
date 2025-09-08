#include "ShadertoyTexture.h"

ShadertoyTexture::ShadertoyTexture(ref<Device> pDevice, const std::string & texturePath, Filter filter, Wrap wrap, bool srgb)
{
    mpTexture = Texture::createFromFile(pDevice, texturePath, filter == Filter::Mipmap, srgb);
    if (!mpTexture) {
        logError("Failed to load texture from file: " + texturePath);
        return;
    }

    Sampler::Desc samplerDesc;
    switch (filter) {
    case Filter::Nearest:
        samplerDesc.setFilterMode(TextureFilteringMode::Point, TextureFilteringMode::Point, TextureFilteringMode::Point);
        break;
    case Filter::Linear:    // there are no mip levels in this case, so we use Point for mip filter
        samplerDesc.setFilterMode(TextureFilteringMode::Linear, TextureFilteringMode::Linear, TextureFilteringMode::Point);
        break;
    case Filter::Mipmap:
        samplerDesc.setFilterMode(TextureFilteringMode::Linear, TextureFilteringMode::Linear, TextureFilteringMode::Linear);
        break;
    default:
        samplerDesc.setFilterMode(TextureFilteringMode::Linear, TextureFilteringMode::Linear, TextureFilteringMode::Linear);
        break;
    }

    switch (wrap) {
    case Wrap::Repeat:
        samplerDesc.setAddressingMode(TextureAddressingMode::Wrap, TextureAddressingMode::Wrap, TextureAddressingMode::Wrap);
        break;
    case Wrap::Clamp:
        samplerDesc.setAddressingMode(TextureAddressingMode::Clamp, TextureAddressingMode::Clamp, TextureAddressingMode::Clamp);
        break;
    default:
        samplerDesc.setAddressingMode(TextureAddressingMode::Wrap, TextureAddressingMode::Wrap, TextureAddressingMode::Wrap);
        break;
    }

    mpSampler = pDevice->createSampler(samplerDesc);

    mTexturePath = texturePath;
    mFilter = filter;
    mWrap = wrap;
    mSrgb = srgb;
}


