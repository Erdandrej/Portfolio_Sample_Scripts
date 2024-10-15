
#include "HeightField.h"
#include "PerlinNoise.h"
#include <algorithm>
#include <iostream>
#include <stdint.h>
#include <vector>
#include <glm/glm.hpp>
#include <stb_image.h>
#include <glm/gtc/matrix_transform.hpp>
#include <random>

using namespace glm;
using std::string;

HeightField::HeightField(void)
    : m_vao(UINT32_MAX)
	, m_indexBuffer(UINT32_MAX)
    , m_vertexBuffer(UINT32_MAX)
	, m_normalBuffer(UINT32_MAX)
	, m_colorBuffer(UINT32_MAX)
    , m_numIndices(0)
	, m_billboardVAO(UINT32_MAX)
	, m_billboardIndicesCount(UINT32_MAX)
	, instanceCount(UINT32_MAX)
{
}

glm::vec3 HeightField::get_color(int r, int g, int b)
{
	return glm::vec3(r / 255.0, g / 255.0, b / 255.0);
}

std::vector<GLfloat> HeightField::generate_noise_map(int width, int height, int gridSize, int octaves, float displacementFactor)
{
	PerlinNoise noise;
	std::vector<float> noiseMap;
	for (int v = 0; v < height; v++) {
		for (int u = 0; u < width; u++) {
			float noiseValue = noise.fullForcePerlin(u, v, gridSize, octaves, displacementFactor, true);
			noiseMap.push_back(noiseValue);
		}
	}
	return noiseMap;
}

std::vector<GLuint> HeightField::generate_indices(int width, int height)
{
	std::vector<GLuint> indices;

	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			int index = i + j * width;

			// Don't create indices for right or top edge
			if (i == width - 1 || j == height - 1) {
				continue;
			}
			else {
				// Top left triangle of square
				indices.push_back(index + width);
				indices.push_back(index);
				indices.push_back(index + width + 1);

				// Bottom right triangle of square
				indices.push_back(index + 1);
				indices.push_back(index + 1 + width);
				indices.push_back(index);
			}
		}
	}

	return indices;
}

std::vector<GLfloat> HeightField::generate_vertices(int size, int width, int height, const std::vector<float>& noise_map , float maxHeight, float waterHeight, bool easeNoise)
{
	std::vector<GLfloat> vertices;
	float stepX = size / width;
	float stepZ = size / height;

	for (int z = 0; z < height; z++) {
		for (int x = 0; x < width; x++) {
			// X coordinate
			float xm = x * stepX - size / 2;
			vertices.push_back(xm);

			// Y coordinate
			float noiseValue = noise_map[x + z * width];
			// Apply cubic easing to noise if desired
			if (easeNoise) noiseValue = std::pow(noiseValue * 1.1, 3);
			// Prevent vertex from being underwater
			float ym = std::fmax(noiseValue * maxHeight, waterHeight * 0.5 * maxHeight);
			vertices.push_back(ym);

			// Z coordinate
			float zm = z * stepZ - size / 2;
			vertices.push_back(zm);
		}
	}
	return vertices;
}

std::vector<GLfloat> HeightField::generate_normals(const std::vector<GLuint>& indices, const std::vector<GLfloat>& vertices)
{
	std::vector<GLfloat> normals;
	glm::vec3 normal;
	std::vector<glm::vec3> verts;

	// Get the vertices of each triangle in mesh
	// For each group of indices
	for (int i = 0; i < indices.size(); i += 3) {

		// Get the vertices (point) for each index
		for (int j = 0; j < 3; j++) {
			int index = indices[i + j] * 3;
			verts.push_back(glm::vec3(vertices[index], vertices[index + 1], vertices[index + 2]));
		}

		// Get vectors of two edges of triangle
		glm::vec3 U = verts[i + 1] - verts[i];
		glm::vec3 V = verts[i + 2] - verts[i];

		// Calculate normal
		normal = glm::normalize(-glm::cross(U, V));
		normals.push_back(normal.x);
		normals.push_back(normal.y);
		normals.push_back(normal.z);
	}
	return normals;
}

std::vector<GLfloat> HeightField::generate_colors(const std::vector<GLfloat>& vertices, float maxHeight, float waterHeight)
{
	std::vector<float> colors;
	std::vector<terrainColor> terrainColors;
	glm::vec3 color = get_color(255, 255, 255);

	terrainColors.push_back(terrainColor(waterHeight * 0.5, get_color(60, 95, 190)));	// Deep water
	terrainColors.push_back(terrainColor(waterHeight, get_color(60, 100, 190)));		// Shallow water
	terrainColors.push_back(terrainColor(0.15, get_color(210, 215, 130)));				// Sand
	terrainColors.push_back(terrainColor(0.30, get_color(95, 165, 30)));				// Grass 1
	terrainColors.push_back(terrainColor(0.40, get_color(65, 115, 20)));				// Grass 2
	terrainColors.push_back(terrainColor(0.50, get_color(90, 65, 60)));					// Rock 1
	terrainColors.push_back(terrainColor(0.80, get_color(75, 60, 55)));					// Rock 2
	terrainColors.push_back(terrainColor(1.00, get_color(255, 255, 255)));				// Ice

	for (int i = 1; i < vertices.size(); i += 3) {
		for (int j = 0; j < terrainColors.size(); j++) {
			if (vertices[i] <= terrainColors[j].height * maxHeight) {
				color = terrainColors[j].color;
				break;
			}
		}
		colors.push_back(color.r);
		colors.push_back(color.g);
		colors.push_back(color.b);
	}

	return colors;
}

std::vector<GLfloat> HeightField::generate_grass_positions(const std::vector<GLfloat>& vertices, int grassFrequency, float grassMinAltitude, float grassMaxAltitude)
{
	std::vector<GLfloat> grassPositions;
	std::default_random_engine generator;
	std::uniform_real_distribution<float> distribution(0.0, 1.0);

	for (size_t i = 0; i < vertices.size(); i += 3) 
	{
		GLfloat x = vertices[i];
		GLfloat y = vertices[i + 1];
		GLfloat z = vertices[i + 2];

		if (grassMinAltitude <= y && y <= grassMaxAltitude)
		{
			float randomValue = distribution(generator);
			// frequency of spawing grass
			if (randomValue < 1.0 / grassFrequency)
			{
				grassPositions.push_back(x);
				grassPositions.push_back(y);
				grassPositions.push_back(z);
				grassPositions.push_back(i);
			}
		}

	}
	return grassPositions;
}

void HeightField::generate_grass(const std::vector<GLfloat>& grassPositions, const std::vector<GLfloat>& normals) {
	std::vector<GLfloat> vertices = {
		// quad vertices for a grass shape
		-0.5f, 0.0f, 0.0f,
		 0.5f, 0.0f, 0.0f,
		-0.25f, 1.0f, 0.0f,
		 0.25f, 1.0f, 0.0f
	};

	std::vector<GLuint> indices = {
		0, 1, 2,
		1, 3, 2
	};

	std::vector<GLfloat> instanceOffsets;
	std::vector<GLfloat> instanceNormals;
	for (size_t i = 0; i < grassPositions.size(); i += 4) {
		instanceOffsets.push_back(grassPositions[i]);
		instanceOffsets.push_back(grassPositions[i+ 1] - 1.1f); //adjustment so the grass doesn't float above the terrain
		instanceOffsets.push_back(grassPositions[i + 2]);

		int normalIndex = grassPositions[i + 3];
		instanceNormals.push_back(normals[normalIndex]);
		instanceNormals.push_back(normals[normalIndex + 1]);
		instanceNormals.push_back(normals[normalIndex + 2]);
	}

	GLuint vao, vbo, ebo, instanceVBO, normalVBO;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &instanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, instanceOffsets.size() * sizeof(GLfloat), instanceOffsets.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribDivisor(1, 1);

	glGenBuffers(1, &normalVBO);
	glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
	glBufferData(GL_ARRAY_BUFFER, instanceNormals.size() * sizeof(GLfloat), instanceNormals.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(2);
	glVertexAttribDivisor(2, 1);

	glBindVertexArray(0);

	m_billboardVAO = vao;
	m_billboardIndicesCount = indices.size();
	instanceCount = grassPositions.size();
}

// render grass
void HeightField::render_grass()
{
	if (m_billboardVAO == UINT32_MAX) {
		std::cout << "No billboard vertex array is generated, cannot draw anything.\n";
		return;
	}

	glBindVertexArray(m_billboardVAO);
	glDrawElementsInstanced(GL_TRIANGLES, m_billboardIndicesCount, GL_UNSIGNED_INT, 0, instanceCount); //instanceCount = number of grass
	glBindVertexArray(0);
}

void HeightField::generate_mesh(int size, int tesselation, int gridSize, int octaves, float displacementFactor, float maxHeight, float waterHeight, bool easeNoise, int grassFrequency, float grassMinAltitude, float grassMaxAltitude)
{
	int width = tesselation;
	int height = tesselation;

	///////////////////////////////////////////////
	// Mesh generation
	//////////////////////////////////////////////
	std::vector<float> noise_map = generate_noise_map(width, height, gridSize, octaves, displacementFactor);
	std::vector<GLuint> indices = generate_indices(width, height);
	std::vector<GLfloat> vertices = generate_vertices(size, width, height, noise_map, maxHeight, waterHeight, easeNoise);
	std::vector<GLfloat> normals = generate_normals(indices, vertices);
	std::vector<GLfloat> colors = generate_colors(vertices, maxHeight, waterHeight);

	m_numIndices = (GLuint)indices.size();

	///////////////////////////////////////////////
	// Grass generation
	//////////////////////////////////////////////
	std::vector<GLfloat> grassPositions = generate_grass_positions(vertices, grassFrequency, grassMinAltitude * maxHeight, grassMaxAltitude * maxHeight);
	generate_grass(grassPositions, normals);

	///////////////////////////////////////////////
	// Buffer generation and binding
	//////////////////////////////////////////////

	// Create and bind Vertex Array Object (m_vao), Index Buffer Object (m_indexBuffer), and Vertex Buffer Object (m_vertexBuffer),  Normal Buffer Object (m_normalBuffer), Color Buffer Object (m_colorBuffer)
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	// IBO
	glGenBuffers(1, &m_indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

	// VBO
	glGenBuffers(1, &m_vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);

	// NBO
	glGenBuffers(1, &m_normalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(1);

	// CBO
	glGenBuffers(1, &m_colorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_colorBuffer);
	glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(GLfloat), colors.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(2);

	// Unbind VAO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void HeightField::render_triangles(void)
{
	if(m_vao == UINT32_MAX)
	{
		std::cout << "No vertex array is generated, cannot draw anything.\n";
		return;
	}

	glBindVertexArray(m_vao);
	glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}