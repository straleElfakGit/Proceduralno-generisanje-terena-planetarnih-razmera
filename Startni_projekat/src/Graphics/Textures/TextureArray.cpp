#include "TextureArray.h"

std::unique_ptr<TextureArray> TextureArray::CreateUnique(const std::vector<const char*>& textures, GLuint slot)
{
    return std::make_unique<TextureArray>(textures, slot);
}

TextureArray::TextureArray(const std::vector<const char*>& textures, GLuint slot)
{
    LOG_FUNC();

    unit = slot;

    glGenTextures(1, &ID);
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D_ARRAY, ID);

    stbi_set_flip_vertically_on_load(true);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    int width, height, numColCh;
    GLenum format = GL_RGBA;

    unsigned char* firstBytes = stbi_load(textures[0], &width, &height, &numColCh, 0);
    if (!firstBytes)
    {
        std::cerr << "TextureArray: failed to load first texture '" << textures[0] << "'" << std::endl;
        return;
    }

    if (numColCh == 3)
        format = GL_RGB;
    else if (numColCh == 4)
        format = GL_RGBA;
    else if (numColCh == 1)
        format = GL_RED;
    else
    {
        std::cerr << "TextureArray: unsupported channel count in '" << textures[0] << "'" << std::endl;
        stbi_image_free(firstBytes);
        return;
    }

    int layerCount = textures.size();
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, format, width, height, layerCount, 0, format, GL_UNSIGNED_BYTE, nullptr);

    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, width, height, 1, format, GL_UNSIGNED_BYTE, firstBytes);
    stbi_image_free(firstBytes);

    for (size_t i = 1; i < textures.size(); i++)
    {
        int w, h, ch;
        unsigned char* bytes = stbi_load(textures[i], &w, &h, &ch, 0);

        if (!bytes)
        {
            std::cerr << "TextureArray: failed to load '" << textures[i] << "'" << std::endl;
            continue;
        }

        if (w != width || h != height || ch != numColCh)
        {
            std::cerr << "TextureArray: dimension/channel mismatch in '" << textures[i] << "'. All textures must be identical in size/format!" << std::endl;
            stbi_image_free(bytes);
            continue;
        }

        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, width, height, 1, format, GL_UNSIGNED_BYTE, bytes);
        stbi_image_free(bytes);
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

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
    GLuint texUni = shader.GetUniformLocation(uniform);
    shader.Activate();
    glUniform1i(texUni, unit);
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