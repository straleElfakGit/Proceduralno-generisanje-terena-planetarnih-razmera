#ifndef SHADER_CLASS_H
#define SHADER_CLASS_H

#include"GLResource.h"
#include<glad/glad.h>
#include<string>
#include<fstream>
#include<sstream>
#include<iostream>
#include<cerrno>
#include<glm/glm.hpp>
#include<glm/gtc/type_ptr.hpp>
#include <unordered_map>

std::string get_file_contents(const char* filename);

class Shader : public GLResource
{
private: 

	mutable std::unordered_map<std::string, int> uniformLocationCache;

	void PrintError(GLuint shader);
	GLuint CreateShader(GLenum type, const char* shaderSource);

protected:
	virtual void DeleteSpecific() override;

public:
	Shader(const char* vertexFile, const char* fragmentFile);
	~Shader();

	virtual void Bind() const override;
	virtual void Unbind() const override;
	void Activate() const;
	GLint GetUniformLocation(const std::string& name) const;

	void setBool(const std::string& name, bool value) const;
	void setInt(const std::string& name, int value) const;
	void setFloat(const std::string& name, float value) const;
	void setMatrix(const std::string& name, const glm::mat4& matrix) const;

    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec2(const std::string& name, float x, float y) const;
	void setVec3(const std::string& name, const glm::vec3& value) const;
	void setVec3(const std::string& name, float x, float y, float z) const;
	void setVec4(const std::string& name, const glm::vec4& value) const;
	void setVec4(const std::string& name, float x, float y, float z, float w) const;
	void setMat2(const std::string& name, const glm::mat2& mat) const;
    void setMat3(const std::string& name, const glm::mat3& mat) const;
	void setMat4(const std::string& name, const glm::mat4& mat) const;
};
#endif