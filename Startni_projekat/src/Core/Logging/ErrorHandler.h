#ifndef ERROR_HANDLIG_H
#define ERROR_HANDLIG_H

#include <glad/glad.h>

#define ASSERT_MSG(condition, message) \
    if (!(condition)) { \
        std::cerr << "[ASSERTION FAILED]: " << message << "\n" \
                  << "File: " << __FILE__ << "\nLine: " << __LINE__ << std::endl; \
        std::abort(); \
    }

#define ASSERT(x) if (!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
	x;\
	ASSERT(GLLogCall(#x, __FILE__, __LINE__))

void GLClearError();
bool GLLogCall(const char* funciton, const char* file, int line);

#endif //ERROR_HANDLIG_H