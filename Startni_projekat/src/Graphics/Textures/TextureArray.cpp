#include "TextureArray.h"

#ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_TEXTURE_MAX_ANISOTROPY_EXT     0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif

std::unique_ptr<TextureArray> TextureArray::CreateUnique(const std::vector<const char*>& textures, GLuint slot, bool srgb)
{
    return std::make_unique<TextureArray>(textures, slot, srgb);
}

bool TextureArray::UploadLayer(const char* path, int layer, int expectedW, int expectedH)
{
    int w, h, ch;
    unsigned char* bytes = stbi_load(path, &w, &h, &ch, 4);
    if (!bytes)
    {
        std::cerr << "TextureArray: failed to load '" << path << "'" << std::endl;
        return false;
    }

    if (w != expectedW || h != expectedH)
    {
        std::cerr << "TextureArray: size mismatch in '" << path << "' (" << w << "x" << h
            << ", expected " << expectedW << "x" << expectedH << ")" << std::endl;
        stbi_image_free(bytes);
        return false;
    }

    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, layer, w, h, 1, GL_RGBA, GL_UNSIGNED_BYTE, bytes);
    stbi_image_free(bytes);
    return true;

}

void TextureArray::SetAnisotropy(float level)
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D_ARRAY, ID);
    GLfloat maxAniso = 1.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
    glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY_EXT, std::min(level, maxAniso));
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

TextureArray::TextureArray(const std::vector<const char*>& textures, GLuint slot, bool srgb)
{
    LOG_FUNC();

    unit = slot;

    if (textures.empty())
    {
        std::cerr << "TextureArray: empty texture list" << std::endl;
        return;
    }
    stbi_set_flip_vertically_on_load(true);

    int width = 0, height = 0, ch = 0;
    if (!stbi_info(textures[0], &width, &height, &ch))
    {
        std::cerr << "TextureArray: failed to read first texture '" << textures[0] << "'" << std::endl;
        return;
    }

    const int layerCount = static_cast<int>(textures.size());
    const int levels = 1 + static_cast<int>(std::floor(std::log2(static_cast<float>(std::max(width, height)))));

    glGenTextures(1, &ID);
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D_ARRAY, ID);

    glTexStorage3D(GL_TEXTURE_2D_ARRAY, levels, srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8, width, height, layerCount);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for (int i = 0; i < layerCount; ++i)
        UploadLayer(textures[i], i, width, height);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

    GLfloat maxAniso = 1.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
    glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY_EXT, std::min(16.0f, maxAniso));

    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

}

TextureArray::~TextureArray()
{
    LOG_FUNC();
    Delete();
}

void TextureArray::texUnit(const Shader& shader, const char* uniform, GLuint unit)
{
    GLint texUni = shader.GetUniformLocation(uniform);
    shader.Activate();
    if (texUni >= 0)
        glUniform1i(texUni, static_cast<GLint>(unit));
    else
        std::cerr << "TextureArray: uniform '" << uniform << "' not found" << std::endl;

}

void TextureArray::Bind() const
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D_ARRAY, ID);
}

void TextureArray::Unbind() const
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void TextureArray::DeleteSpecific()
{
    glDeleteTextures(1, &ID);
}