#pragma once

#include "Angel.h"
#include <vector>
#include "stb_image.h"

class Sky 
{

private:
	mat4 viewMatrix;
	mat4 projection;
	
	float skyboxVertices[108]{
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
	};;

	std::vector<std::string> faces
	{
		"texture/ely_hills/hills_ft.tga",
		"texture/ely_hills/hills_bk.tga",
		"texture/ely_hills/hills_up.tga",
		"texture/ely_hills/hills_dn.tga",
		"texture/ely_hills/hills_rt.tga",
		"texture/ely_hills/hills_lf.tga"
	};

	GLuint skyboxVAO, skyboxVBO;
	GLuint programId;
	GLuint cubemapTexture;

	void buildSky()
	{
		programId = InitShader("sky_vShader.glsl", "sky_fShader.glsl");

		glGenVertexArrays(1, &skyboxVAO);
		glGenBuffers(1, &skyboxVBO);
		glBindVertexArray(skyboxVAO);
		glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

		Projection = glGetUniformLocation(programId, "projMatrix");
		ViewMatrix = glGetUniformLocation(programId, "viewMatrix");
	}

	GLuint loadCubemap(std::vector<std::string> faces)
	{
		unsigned int textureID;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

		int width, height, nrChannels;
		for (unsigned int i = 0; i < faces.size(); i++)
		{
			unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
			if (data)
			{
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
				stbi_image_free(data);
			}
			else
			{
				std::cout << "Cubemap texture failed to load at path: " << faces[i].c_str() << std::endl;
				stbi_image_free(data);
			}
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		return textureID;
	}

public:

	GLuint ViewMatrix;
	GLuint Projection;

	Sky() 
	{
		skyboxVAO = 0;
		skyboxVBO = 0;

		buildSky();
		cubemapTexture = loadCubemap(faces);
	};

	void setMatrix(mat4 modelViewMatrix, mat4 projection_)
	{
		viewMatrix = modelViewMatrix;
		projection = projection_;
	}

	void drawsky() 
	{
		glUseProgram(programId);

		glDepthFunc(GL_LEQUAL);

		glBindVertexArray(skyboxVAO);

		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(programId, "skybox"), 0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

		glUniformMatrix4fv(ViewMatrix, 1, GL_TRUE, viewMatrix);
		glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);

		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); // set depth function back to default

	}
};