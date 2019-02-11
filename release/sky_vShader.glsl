#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

// uniform mat4 modelMatrix;
uniform mat4 projMatrix;
uniform mat4 viewMatrix;

void main()
{
	TexCoords = aPos;
    vec4 pos = projMatrix * mat4(mat3(viewMatrix))*  vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}