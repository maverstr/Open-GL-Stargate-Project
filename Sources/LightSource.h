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

enum lightType {
	POINTLIGHT,
	DIRECTIONALLIGHT,
	SPOTLIGHT
};


// An abstract LightSource class that creates the light source, its VAO (or uses the similar ones from other light sources) and draws them
class LightSource
{
public:
	lightType type;
	glm::vec4 Position;
	glm::vec3 Color;
	glm::vec3 Ambient;
	glm::vec3 Diffuse;
	glm::vec3 Specular;
	int AttenuationBool; //workaround as it has to be int in the shaders
	GLfloat AttenuationConstant;
	GLfloat AttenuationLinear;
	GLfloat AttenuationQuadratic;
	int SpotlightBool;
	glm::vec3 Direction;
	GLfloat InnerCutOff;
	GLfloat OuterCutOff;
	GLfloat Size = 10.0f;
	GLuint VAO = 0;

	//Only specifies color, WITH ATTENUATION
	LightSource(int* lightCounter, lightType type, glm::vec3 position, glm::vec3 color, GLfloat constant, GLfloat linear, GLfloat quadratic, GLfloat size, GLuint VAO)
	{
		if (type == POINTLIGHT) {
			this->Position = glm::vec4(position, 1.0f);
		}
		else if (type == DIRECTIONALLIGHT) {
			this->Position = glm::vec4(position, 0.0f);
		}
		this->Color = color;
		this->Size = size;
		//The VAO has to be created once but can be used again for other similar light sources
		if (VAO != 0) {
			this->VAO = VAO;
		}
		else {
			this->VAO = createVAO();
		}
		this->Ambient = this->Color;
		this->Diffuse = this->Color;
		this->Specular = this->Color;

		this->AttenuationBool = 1;
		this->AttenuationConstant = constant;
		this->AttenuationLinear = linear;
		this->AttenuationQuadratic = quadratic;
		this->SpotlightBool = 0;

		this->type = type;
		(*lightCounter)++;
	}

	//Only specifies color, WITHOUT ATTENUATION
	LightSource(int* lightCounter, lightType type, glm::vec3 position, glm::vec3 color, GLfloat size, GLuint VAO)
	{
		if (type == POINTLIGHT)
			this->Position = glm::vec4(position, 1.0f);
		else if (type == DIRECTIONALLIGHT)
			this->Position = glm::vec4(position, 0.0f);
		this->Color = color;
		this->Size = size;
		//The VAO has to be created once but can be used again for other similar light sources
		if (VAO != 0) {
			this->VAO = VAO;
		}
		else {
			this->VAO = createVAO();
		}
		this->Ambient = this->Color;
		this->Diffuse = this->Color;
		this->Specular = this->Color;

		this->AttenuationBool = 0;
		this->AttenuationConstant = 0.0f;
		this->AttenuationLinear = 0.0f;
		this->AttenuationQuadratic = 0.0f;
		this->SpotlightBool = 0;

		this->type = type;
		(*lightCounter)++;
	}

	//Specifies ambient diffuse and specular components, WITH ATTENUATION
	LightSource(int* lightCounter, lightType type, glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, GLfloat constant, GLfloat linear, GLfloat quadratic, GLfloat size, GLuint VAO)
	{
		if (type == POINTLIGHT)
			this->Position = glm::vec4(position, 1.0f);
		else if (type == DIRECTIONALLIGHT)
			this->Position = glm::vec4(position, 0.0f);
		this->Size = size;
		//The VAO has to be created once but can be used again for other similar light sources
		if (VAO != 0) {
			this->VAO = VAO;
		}
		else {
			this->VAO = createVAO();
		}
		this->Ambient = ambient;
		this->Diffuse = diffuse;
		this->Specular = specular;
		this->Color = ambient;

		this->AttenuationBool = 1;
		this->AttenuationConstant = constant;
		this->AttenuationLinear = linear;
		this->AttenuationQuadratic = quadratic;
		this->SpotlightBool = 0;

		this->type = type;
		(*lightCounter)++;
	}

	//Specifies ambient diffuse and specular components, WITHTOUT ATTENUATION
	LightSource(int* lightCounter, lightType type, glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, GLfloat size, GLuint VAO)
	{
		if (type == POINTLIGHT)
			this->Position = glm::vec4(position, 1.0f);
		else if (type == DIRECTIONALLIGHT)
			this->Position = glm::vec4(position, 0.0f);
		this->Size = size;
		//The VAO has to be created once but can be used again for other similar light sources
		if (VAO != 0) {
			this->VAO = VAO;
		}
		else {
			this->VAO = createVAO();
		}
		this->Ambient = ambient;
		this->Diffuse = diffuse;
		this->Specular = specular;
		this->Color = ambient;

		this->AttenuationBool = 0;
		this->AttenuationConstant = 0.0f;
		this->AttenuationLinear = 0.0f;
		this->AttenuationQuadratic = 0.0f;
		this->SpotlightBool = 0;

		this->type = type;
		(*lightCounter)++;
	}
	
	//Specifies ambient diffuse and specular components, WITH ATTENUATION, CASE OF SPOTLIGHT
	LightSource(int* lightCounter, lightType type, glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, GLfloat constant, GLfloat linear, GLfloat quadratic, glm::vec3 direction, GLfloat innerCutOff, GLfloat outerCutOff, GLfloat size, GLuint VAO)
	{
		if (type == POINTLIGHT)
			this->Position = glm::vec4(position, 1.0f);
		else if (type == DIRECTIONALLIGHT)
			this->Position = glm::vec4(position, 0.0f);
		else if (type == SPOTLIGHT) {
			this->Position = glm::vec4(position, 1.0f);
			this->Direction = direction;
			this->InnerCutOff = glm::cos(glm::radians(innerCutOff)); //let's calculate directly the cosine
			this->OuterCutOff = glm::cos(glm::radians(outerCutOff));
			this->SpotlightBool = 1;
		}
		this->Size = size;
		//The VAO has to be created once but can be used again for other similar light sources
		if (VAO != 0) {
			this->VAO = VAO;
		}
		else {
			this->VAO = createVAO();
		}
		this->Ambient = ambient;
		this->Diffuse = diffuse;
		this->Specular = specular;
		this->Color = ambient;

		this->AttenuationBool = 1;
		this->AttenuationConstant = constant;
		this->AttenuationLinear = linear;
		this->AttenuationQuadratic = quadratic;

		this->type = type;
		(*lightCounter)++;
	}
	
	//Specifies ambient diffuse and specular components, WITHOUT ATTENUATION, CASE OF SPOTLIGHT
	LightSource(int* lightCounter, lightType type, glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, glm::vec3 direction, GLfloat innerCutOff, GLfloat outerCutOff, GLfloat size, GLuint VAO)
	{
		if (type == POINTLIGHT)
			this->Position = glm::vec4(position, 1.0f);
		else if (type == DIRECTIONALLIGHT)
			this->Position = glm::vec4(position, 0.0f);
		else if (type == SPOTLIGHT) {
			this->Position = glm::vec4(position, 1.0f);
			this->Direction = direction;
			this->InnerCutOff = glm::cos(glm::radians(innerCutOff)); //let's calculate directly the cosine
			this->OuterCutOff = glm::cos(glm::radians(outerCutOff));
			this->SpotlightBool = 1;
		}
		this->Size = size;
		//The VAO has to be created once but can be used again for other similar light sources
		if (VAO != 0) {
			this->VAO = VAO;
		}
		else {
			this->VAO = createVAO();
		}
		this->Ambient = ambient;
		this->Diffuse = diffuse;
		this->Specular = specular;
		this->Color = ambient;

		this->AttenuationBool = 0;
		this->AttenuationConstant = 0.0f;
		this->AttenuationLinear = 0.0f;
		this->AttenuationQuadratic = 0.0f;

		this->type = type;
		(*lightCounter)++;
	}

	//Specifies color only, WITH ATTENUATION, CASE OF SPOTLIGHT
	LightSource(int* lightCounter, lightType type, glm::vec3 position, glm::vec3 color, GLfloat constant, GLfloat linear, GLfloat quadratic, glm::vec3 direction, GLfloat innerCutOff, GLfloat outerCutOff, GLfloat size, GLuint VAO)
	{
		if (type == POINTLIGHT)
			this->Position = glm::vec4(position, 1.0f);
		else if (type == DIRECTIONALLIGHT)
			this->Position = glm::vec4(position, 0.0f);
		else if (type == SPOTLIGHT) {
			this->Position = glm::vec4(position, 1.0f);
			this->Direction = direction;
			this->InnerCutOff = glm::cos(glm::radians(innerCutOff)); //let's calculate directly the cosine
			this->OuterCutOff = glm::cos(glm::radians(outerCutOff));
			this->SpotlightBool = 1;
		}
		this->Size = size;
		//The VAO has to be created once but can be used again for other similar light sources
		if (VAO != 0) {
			this->VAO = VAO;
		}
		else {
			this->VAO = createVAO();
		}
		this->Ambient = color;
		this->Diffuse = color;
		this->Specular = color;
		this->Color = color;

		this->AttenuationBool = 1;
		this->AttenuationConstant = constant;
		this->AttenuationLinear = linear;
		this->AttenuationQuadratic = quadratic;

		this->type = type;
		(*lightCounter)++;
	}

	//Specifies color only, WITHOUT ATTENUATION, CASE OF SPOTLIGHT
	LightSource(int* lightCounter, lightType type, glm::vec3 position, glm::vec3 color, glm::vec3 direction, GLfloat innerCutOff, GLfloat outerCutOff, GLfloat size, GLuint VAO)
	{
		if (type == POINTLIGHT)
			this->Position = glm::vec4(position, 1.0f);
		else if (type == DIRECTIONALLIGHT)
			this->Position = glm::vec4(position, 0.0f);
		else if (type == SPOTLIGHT) {
			this->Position = glm::vec4(position, 1.0f);
			this->Direction = direction;
			this->InnerCutOff = glm::cos(glm::radians(innerCutOff)); //let's calculate directly the cosine
			this->OuterCutOff = glm::cos(glm::radians(outerCutOff));
			this->SpotlightBool = 1;
		}
		this->Size = size;
		//The VAO has to be created once but can be used again for other similar light sources
		if (VAO != 0) {
			this->VAO = VAO;
		}
		else {
			this->VAO = createVAO();
		}
		this->Ambient = color;
		this->Diffuse = color;
		this->Specular = color;
		this->Color = color;

		this->AttenuationBool = 0;
		this->AttenuationConstant = 0.0f;
		this->AttenuationLinear = 0.0f;
		this->AttenuationQuadratic = 0.0f;

		this->type = type;
		(*lightCounter)++;
	}

	void draw(Shader shader, glm::mat4 modelMatrix, glm::mat4 viewMatrix, glm::mat4 projectionMatrix, glm::vec3 viewPos) {
		shader.use();
		if (this->type == POINTLIGHT || this->type == SPOTLIGHT) { //only point lights are drawn
			glm::mat4 model = glm::translate(modelMatrix, glm::vec3(this->Position));
			model = glm::scale(model, glm::vec3(this->Size, this->Size, this->Size));
			shader.setMatrix4("model", model);
			shader.setMatrix4("projection", projectionMatrix);
			shader.setMatrix4("view", viewMatrix);
			shader.setVector3f("color", this->Color / (max(max(this->Color.x, this->Color.y), this->Color.z))); //need to give full intensity for a color source to be bright...
														//so dividing by the highest component to get 1 as max component
			shader.setVector3f("thisLight.position", this->Position);
			shader.setVector3f("viewPos", viewPos);
			shader.setInteger("thisLight.attenuationBool", this->AttenuationBool);
			shader.setFloat("thisLight.constant", this->AttenuationConstant);
			shader.setFloat("thisLight.linear", this->AttenuationLinear /50); //better when light source is not as attenuated as what it lights up... 
			shader.setFloat("thisLight.quadratic", this->AttenuationQuadratic /100);//... so attenuation is reduced
			glBindVertexArray(this->VAO);
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0); // draw the light
			glBindVertexArray(0);
		}
	}

	void updatePosition(glm::vec3 pos) {
		this->Position.x = pos.x;
		this->Position.y = pos.y;
		this->Position.z = pos.z;
	}

	void updateFlashLightDirection(glm::vec3 pos) {
		this->Direction = pos;
	}

	//Need to make sure the shader is "use()" before !!!!! and does not set textures !!!
	void setModelShaderLightParameters(Shader shader, int lightNumber) {
		string number = to_string(lightNumber);

		shader.setVector3f(("light[" + number + "].ambient").c_str(), this->Ambient);
		shader.setVector3f(("light[" + number + "].diffuse").c_str(), this->Diffuse);
		shader.setVector3f(("light[" + number + "].specular").c_str(), this->Specular);
		shader.setVector4f(("light[" + number + "].position").c_str(), this->Position);
		shader.setInteger(("light[" + number + "].spotlight").c_str(), this->SpotlightBool);
		shader.setFloat(("light[" + number + "].innerCutOff").c_str(), this->InnerCutOff);
		shader.setFloat(("light[" + number + "].outerCutOff").c_str(), this->OuterCutOff);
		shader.setVector3f(("light[" + number + "].direction").c_str(), this->Direction);
		shader.setInteger(("light[" + number + "].attenuationBool").c_str(), this->AttenuationBool);
		shader.setFloat(("light[" + number + "].constant").c_str(), this->AttenuationConstant);
		shader.setFloat(("light[" + number + "].linear").c_str(), this->AttenuationLinear);
		shader.setFloat(("light[" + number + "].quadratic").c_str(), this->AttenuationQuadratic);		
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