#ifndef DELETEABLE_H
#define DELETEABLE_H

#include<glad/glad.h>

class Deleteable
{
private:
    bool deleted;

protected:

    virtual void DeleteSpecific() = 0;

public:
    Deleteable();
    virtual ~Deleteable();

    void Delete();
};

#endif // DELETEABLE_H
