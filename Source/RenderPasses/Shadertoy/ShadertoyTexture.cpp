#include "ShadertoyTexture.h"
#include "Utils/Image/Bitmap.h"
Falcor::ref<Falcor::Texture> createCubeMap(Falcor::ref<Falcor::Device> pDevice, const std::vector<std::string>& facePaths)
{
    assert(facePaths.size() == 6);
    std::vector<Falcor::Bitmap::UniqueConstPtr> faceBitmaps(6);
    for (size_t i = 0; i < 6; ++i)
        faceBitmaps[i] = Bitmap::createFromFile(facePaths[i], true);
    
    // Check that all faces were loaded and have the same dimensions and format
    uint32_t width = faceBitmaps[0]->getWidth();
    uint32_t height = faceBitmaps[0]->getHeight();
    Falcor::ResourceFormat format = faceBitmaps[0]->getFormat();
    for (size_t i = 1; i < 6; ++i) {
        if (!faceBitmaps[i] || faceBitmaps[i]->getWidth() != width || faceBitmaps[i]->getHeight() != height || faceBitmaps[i]->getFormat() != format) {
            logError("Failed to create cubemap: face images have different dimensions or formats.");
            return nullptr;
        }
    }

    // Create the cubemap texture
    std::vector<uint8_t> cubeData(width * height * Falcor::getFormatBytesPerBlock(format) * 6);
    // Copy face data into the cubemap data
    for (size_t i = 0; i < 6; ++i) {
        size_t faceSize = width * height * Falcor::getFormatBytesPerBlock(format);
        std::memcpy(cubeData.data() + i * faceSize, faceBitmaps[i]->getData(), faceSize);
    }
    return pDevice->createTextureCube(width, height, format, 1, 1, cubeData.data());
}

ShadertoyTexture::ShadertoyTexture(ref<Device> pDevice, const std::string & texturePath, Filter filter, Wrap wrap, bool srgb)
{
    // check if the file exists
    if (!std::filesystem::exists(texturePath)) {
        logError("Texture file does not exist: " + texturePath);
        return;
    }
    // check if there are files with suffixes for cubemap faces (<name>_1 to <name>_5)
    bool isCubemap = true;
    std::vector<std::string> facePaths;
    facePaths.push_back(texturePath);
    for (int i = 1; i < 6; ++i) {
        std::string facePath = texturePath;
        size_t dotPos = facePath.find_last_of('.');
        if (dotPos != std::string::npos) {
            facePath.insert(dotPos, "_" + std::to_string(i));
        } else {
            facePath += "_" + std::to_string(i);
        }
        if (!std::filesystem::exists(facePath)) {
            isCubemap = false;
            break;
        }
        facePaths.push_back(facePath);
    }

    if (isCubemap) {
        mpTexture = createCubeMap(pDevice, facePaths);
    } else {
        mpTexture = Texture::createFromFile(pDevice, texturePath, filter == Filter::Mipmap, srgb);
    }
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

