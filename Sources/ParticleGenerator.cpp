/*******************************************************************
** This code is part of Breakout.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/


//used in this project with some tweaks

#include "ParticleGenerator.h"
#include <iostream>

ParticleGenerator::ParticleGenerator(Shader shader, GLuint amount)
	: shader(shader), amount(amount)
{
	this->init();
}

void ParticleGenerator::Update(GLfloat dt, glm::vec3 pos, glm::vec3 velocityDirection, GLuint newParticles, glm::vec3 offset)
{
	// Add new particles 
	for (GLuint i = 0; i < newParticles; ++i)
	{
		int unusedParticle = this->firstUnusedParticle();
		this->respawnParticle(this->particles[unusedParticle], pos, velocityDirection, offset);
	}
	// Update all particles
	for (GLuint i = 0; i < this->amount; ++i)
	{
		Particle& p = this->particles[i];
		p.Life -= dt; // reduce life
		if (p.Life > 0.0f)
		{	// particle is alive, thus update
			p.Position -= p.Velocity * dt * (0.5f + rand()%100 /100.0f);
			p.Color.a -= dt * 0.95f * (0.5f + rand() % 100 / 100.0f); //progressive transparency
			p.Color.g += dt * 0.8f * (0.5f + rand() % 100 / 100.0f); //progressive yellow by increasing green up to 0.8
		}
	}
}

// Render all particles
void ParticleGenerator::Draw()
{
	// Use additive blending to give it a 'glow' effect
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	this->shader.use();
	for (Particle particle : this->particles)
	{
		if (particle.Life > 0.0f)
		{
			this->shader.setVector3f("offset", particle.Position);
			this->shader.setVector4f("color", particle.Color);
			glBindVertexArray(this->VAO);
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
			glBindVertexArray(0);
		}
	}
	// Don't forget to reset to default blending mode
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void ParticleGenerator::init()
{
	GLfloat cube_vertices[] = {
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
	GLuint VBO, EBO;
	glGenVertexArrays(1, &this->VAO);
	glBindVertexArray(this->VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_elements), cube_elements, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0); //position
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Create this->amount default particle instances
	for (GLuint i = 0; i < this->amount; ++i)
		this->particles.push_back(Particle());
}

// Stores the index of the last particle used (for quick access to next dead particle)
GLuint lastUsedParticle = 0;
GLuint ParticleGenerator::firstUnusedParticle()
{
	// First search from last used particle, this will usually return almost instantly
	for (GLuint i = lastUsedParticle; i < this->amount; ++i) {
		if (this->particles[i].Life <= 0.0f) {
			lastUsedParticle = i;
			return i;
		}
	}
	// Otherwise, do a linear search
	for (GLuint i = 0; i < lastUsedParticle; ++i) {
		if (this->particles[i].Life <= 0.0f) {
			lastUsedParticle = i;
			return i;
		}
	}
	// All particles are taken, override the first one (note that if it repeatedly hits this case, more particles should be reserved)
	lastUsedParticle = 0;
	return 0;
}

void ParticleGenerator::respawnParticle(Particle& particle, glm::vec3 pos, glm::vec3 velocityDirection, glm::vec3 offset)
{
	GLfloat random1 = ((rand() % 100) - 50) / 200.0f;
	GLfloat random2 = ((rand() % 100) - 50) / 200.0f;
	GLfloat random3 = ((rand() % 100) - 50) / 200.0f;
	glm::vec3 random = glm::vec3(random1, random2, random3);
	GLfloat colorR = 0.5f + (rand() % 50 / 100.0f); // 0.5 to 1
	GLfloat colorG = 0.0f + (rand() % 20 / 100.0f); // 0 to 0.2
	particle.Position = pos + random + offset;
	particle.Color = glm::vec4(1.0f, colorG, 0.0f, 1.0f); //base color is red-ish
	particle.Life = 1.0f;
	particle.Velocity = velocityDirection * 10.0f;
}