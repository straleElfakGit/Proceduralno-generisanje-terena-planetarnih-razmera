#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <string>
#include <unordered_map>
#include <memory>

#include "shaderClass.h"
#include "Textures/CubeMap.h"
#include "Textures/GradientTexture.h"
#include "Textures/Texture.h"
#include "Textures/TextureArray.h"

class ResourceManager
{
private:
	std::unordered_map<std::string, std::unique_ptr<Shader>> shaders;
	std::unordered_map<std::string, std::unique_ptr<CubeMap>> cubeMaps;
	std::unordered_map<std::string, std::unique_ptr<GradientTexture>> gradientTextures;
	std::unordered_map<std::string, std::unique_ptr<Texture>> textures;
	std::unordered_map<std::string, std::unique_ptr<TextureArray>> textureArrays;

	ResourceManager() = default;
	~ResourceManager() { ClearAll(); }

public:
	static ResourceManager& GetInstance()
	{
		static ResourceManager instance;
		return instance;
	}

	ResourceManager(const ResourceManager&) = delete;
	ResourceManager& operator=(const ResourceManager&) = delete;

	Shader* GetOrLoadShader(const std::string& name, const std::string& vertPath, const std::string& fragPath);
	Shader* GetShader(const std::string& name);

	CubeMap* GetOrLoadCubeMap(const std::string& name, const std::array<const char*, 6>& faces, GLuint slot);
	CubeMap* GetCubeMap(const std::string& name);

	GradientTexture* GetOrLoadGradientTexture(const std::string& name, int resolution, GLuint slot);
	GradientTexture* GetGradientTexture(const std::string& name);

	Texture* GetOrLoadTexture(const std::string& name, const char* image, GLenum texType, GLuint slot, GLenum format, GLenum pixelType);
	Texture* GetTexture(const std::string& name);

	TextureArray* GetOrLoadTextureArray(const std::string& name, const std::vector<const char*>& textures, GLuint slot);
	TextureArray* GetTextureArray(const std::string& name);

	void ClearAll();

};

#endif //!RESOURCE_MANAGER_H