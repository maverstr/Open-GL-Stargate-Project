#pragma once
// Std. Includes
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

// GL Includes
#include <glad/glad.h> // Contains all the necessery OpenGL includes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Shader.hpp"


// An abstract LightSource class that creates the light source, its VAO (or uses the similar ones from other light sources) and draws them
class LightSource
{
public:
	glm::vec3 Position;
	glm::vec3 Color;
	GLfloat Size = 10.0f;
	GLuint VAO = 0;

	LightSource( glm::vec3 position, glm::vec3 color, GLfloat size, GLuint VAO)
	{
		this->Position = position;
		this->Color = color;
		this->Size = size;
		//The VAO has to be created once but can be used again for other similar light sources
		if (VAO != 0) {
			this->VAO = VAO;
		}
		else {
			this->VAO = createVAO();
		}
	}


	void draw(Shader shader, glm::mat4 modelMatrix, glm::mat4 viewMatrix, glm::mat4 projectionMatrix) {
		shader.use();
		glm::mat4 model = glm::translate(modelMatrix, this->Position);
		model = glm::scale(model, glm::vec3(this->Size, this->Size, this->Size));
		shader.setMatrix4("model", model);
		shader.setMatrix4("projection", projectionMatrix);
		shader.setMatrix4("view", viewMatrix);
		shader.setVector3f("color", this->Color);
		glBindVertexArray(this->VAO);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0); // draw the light
		glBindVertexArray(0);
	}

	void updatePosition(glm::vec3 pos) {
		this->Position = pos;
	}

	//This function can be called to retrieve the VAO for other smilar light sources
	GLuint getVAO() {
		return this->VAO;
	}

private:
	GLuint createVAO(void) {
		GLfloat light_vertices[] = {
			// front
			-1.0, -1.0,  1.0,
			1.0, -1.0,  1.0,
			1.0,  1.0,  1.0,
			-1.0,  1.0,  1.0,
			// back
			-1.0, -1.0, -1.0,
				1.0, -1.0, -1.0,
			1.0,  1.0, -1.0,
			-1.0,  1.0, -1.0
		};

		GLushort cube_elements[] = {
			// front
			0, 1, 2,
			2, 3, 0,
			// right
			1, 5, 6,
			6, 2, 1,
			// back
			7, 6, 5,
			5, 4, 7,
			// left
			4, 0, 3,
			3, 7, 4,
			// bottom
			4, 5, 1,
			1, 0, 4,
			// top
			3, 2, 6,
			6, 7, 3
		};
		GLuint VBO, VAO, EBO;
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(light_vertices), light_vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_elements), cube_elements, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0); //position
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		return VAO;

	}

};