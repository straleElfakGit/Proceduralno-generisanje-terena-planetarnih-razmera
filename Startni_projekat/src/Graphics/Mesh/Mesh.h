#ifndef  MESH_CLASS_H
#define MESH_CLASS_H

#include <vector>

#include "Buffers/EBO.h"
#include "Buffers/VAO.h"
#include "Camera/Camera.h"
#include "Textures/Texture.h"
#include "Vertex.h"
#include "Materials/Material.h"

class Mesh
{
private:
	std::vector <Vertex> vertices;
	std::vector <GLuint> indices;
	std::vector <Texture> textures;

	VAO<GLuint> VAO;
	VBO<GLuint> VBO;
	EBO<GLuint> EBO;

	Material* material;

	Mesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, std::vector<Texture>& textures, Material* material = nullptr);
	~Mesh();

	void Draw(Shader& shader, Camera& camera);
};

#endif // ! MESH_CLASS_H
