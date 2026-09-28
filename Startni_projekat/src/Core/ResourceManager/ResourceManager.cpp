#include "ResourceManager.h"

Shader* ResourceManager::GetOrLoadShader(const std::string& name, const std::string& vertPath, const std::string& fragPath)
{
	auto it = shaders.find(name);
	if (it != shaders.end())
		return it->second.get();

	LOG_INFO("Loading new Shader: {}", name);

    auto shader = Shader::CreateUniqe(vertPath.c_str(), fragPath.c_str());
    Shader* rawPtr = shader.get();
    shaders[name] = std::move(shader);
    return rawPtr;
}

Shader* ResourceManager::GetShader(const std::string& name)
{
    auto it = shaders.find(name);
    if (it != shaders.end())
        return it->second.get();

    return nullptr;
}

CubeMap* ResourceManager::GetOrLoadCubeMap(const std::string& name, const std::array<const char*, 6>& faces, GLuint slot)
{
    auto it = cubeMaps.find(name);
    if (it != cubeMaps.end())
        return it->second.get();

    LOG_INFO("Loading new Cube map: {}", name);

    auto cubemap = CubeMap::CreateUnique(faces, slot);
    CubeMap* rawPtr = cubemap.get();
    cubeMaps[name] = std::move(cubemap);
    return rawPtr;
}

CubeMap* ResourceManager::GetCubeMap(const std::string& name)
{
    auto it = cubeMaps.find(name);
    if (it != cubeMaps.end())
        return it->second.get();

    return nullptr;
}

GradientTexture* ResourceManager::GetOrLoadGradientTexture(const std::string& name, int resolution, GLuint slot)
{
    auto it = gradientTextures.find(name);
    if (it != gradientTextures.end())
        return it->second.get();

    auto gradientTexture = GradientTexture::CreateUnique(resolution, slot);
    GradientTexture* rawPtr = gradientTexture.get();
    gradientTextures[name] = std::move(gradientTexture);
    return rawPtr;
}

GradientTexture* ResourceManager::GetGradientTexture(const std::string& name)
{
    auto it = gradientTextures.find(name);
    if (it != gradientTextures.end())
        return it->second.get();

    return nullptr;
}

Texture* ResourceManager::GetOrLoadTexture(const std::string& name, const char* image, GLenum texType, GLuint slot, GLenum format, GLenum pixelType)
{
    auto it = textures.find(name);
    if (it != textures.end())
        return it->second.get();

    auto texture = Texture::CreateUnique(image, texType, slot, format, pixelType);
    Texture* rawPtr = texture.get();
    textures[name] = std::move(texture);
    return rawPtr;
}

Texture* ResourceManager::GetTexture(const std::string& name)
{
    auto it = textures.find(name);
    if (it != textures.end())
        return it->second.get();

    return nullptr;
}

void ResourceManager::ClearAll()
{
    cubeMaps.clear();
    gradientTextures.clear();
    textures.clear();
    shaders.clear();
}