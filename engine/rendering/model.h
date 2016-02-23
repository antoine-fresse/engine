#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <utils.h>


struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uv;
};

class Texture
{

};

class Mesh
{
	Mesh(const std::vector<Vertex>& positions, const std::vector<uint32>& indices, const std::vector<std::shared_ptr<Texture>>& shared_ptrs)
		: positions(positions),
		  indices(indices),
		  textures(shared_ptrs),
			VBO(0), VAO(0), EBO(0)
	{
	}

public:
	std::vector<Vertex> positions;
	std::vector<uint32> indices;

	std::vector<std::shared_ptr<Texture>> textures;

	void uploadData();

private:
	

	uint32 VBO, VAO, EBO;

};


