#pragma once

#include <vector>
#include <string>
#include "Angel.h"

struct Camera
{
	mat4 modelMatrix;
	mat4 viewMatrix;

	mat4 lookAt(const vec4& eye, const vec4& at, const vec4& up)
	{
		vec4 n = normalize(eye - at);
		vec4 u = vec4(normalize(cross(up, n)), 0.0);
		vec4 v = vec4(normalize(cross(n, u)), 0.0);
		vec4 t = vec4(0.0, 0.0, 0.0, 1.0);

		return mat4(u, v, n, t)*Translate(-eye);
	}

	mat4 perspective(const GLfloat fovy, const GLfloat aspect,
		const GLfloat zNear, const GLfloat zFar)
	{
		GLfloat top = tan(fovy * M_PI / 180 / 2) * zNear;
		GLfloat right = top * aspect;

		mat4 c = mat4(zNear / right, 0.0, 0.0, 0.0,
			0.0, zNear / top, 0.0, 0.0,
			0.0, 0.0, -(zFar + zNear) / (zFar - zNear), -1.0,
			0.0, 0.0, -(2.0 * zFar*zNear) / (zFar - zNear), 0.0);
		return c;
	}
};