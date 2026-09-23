#ifndef GL_RESOURCE_H
#define GL_RESOURCE_H

#include <glad/glad.h>
#include "Deleteable.h"

class GLResource : public Deleteable
{
protected:
    GLuint ID;

public:
    GLResource();
    virtual ~GLResource();

    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;
};

#endif