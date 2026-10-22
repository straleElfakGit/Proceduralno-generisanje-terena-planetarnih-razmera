#ifndef TEXTURE_ARRAY_H
#define TEXTURE_ARRAY_H

#include <glad/glad.h>
#include <stb/stb_image.h>
#include <memory>
#include <vector>
#include <algorithm>
#include <cmath>

#include "Logging/Logger.h"
#include "GLResource.h"
#include "shaderClass.h"

class TextureArray : public GLResource
{
private:
    GLuint unit;

    static bool UploadLayer(const char* path, int layer, int expectedW, int expectedH);

protected:
    virtual void DeleteSpecific() override;

public:
    static std::unique_ptr<TextureArray> CreateUnique(const std::vector<const char*>& textures, GLuint slot, bool srgb);

    TextureArray(const std::vector<const char*>& textures, GLuint slot, bool srgb);
    ~TextureArray();

    void texUnit(const Shader& shader, const char* uniform, GLuint unit);

    void SetAnisotropy(float level);

    virtual void Bind() const override;
    virtual void Unbind() const override;
};

#endif // !TEXTURE_ARRAY_H

