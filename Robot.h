#pragma once

#include "MatrixStack.h"

#define TORSO_HEIGHT 5.0
#define TORSO_WIDTH 3.0
#define ARM_HEIGHT 3.0
#define LEG_WIDTH  0.5
#define LEG_HEIGHT 3.0
#define ARM_WIDTH  0.5
#define HEAD_HEIGHT 1.5
#define HEAD_WIDTH 1.0

struct Robot
{
	enum
	{
		Torso,
		Head1,
		Head2,
		RightArm,
		LeftArm,
		RightLeg,
		LeftLeg,
		NumJointAngles,
		Quit
	};

	// Joint angles with initial values
	GLfloat theta[NumJointAngles] =
	{
		0.0,    // Torso
		0.0,    // Head1
		0.0,    // Head2
		180.0,    // RightArm
		180.0,    // LeftArm
		180.0,  // RightLeg
		180.0,  // LeftLeg
	};

	GLint angle = Head2;
	MatrixStack mvstack;
	mat4 model_view = mat4(1.0);

	void build()
	{
		model_view = Translate(steve.x, steve.y, steve.z + 4.3)*RotateZ(theta[Torso])*RotateX(90);
		obj_torso();
		mvstack.push(model_view);

		model_view *= (Translate(0.0, 2.5, 0.0) * RotateY(theta[Head1])*RotateZ(theta[Head2]));
		obj_head();

		model_view = mvstack.pop();
		mvstack.push(model_view);
		model_view *= (Translate(0.0, 1.2, -1.5) * RotateY(180));
		obj_left_arm();

		model_view = mvstack.pop();
		mvstack.push(model_view);
		model_view *= (Translate(0.0, 1.2, 1.5) * RotateY(180));
		obj_right_arm();

		model_view = mvstack.pop();
		mvstack.push(model_view);
		model_view *= (Translate(0.0, -1.4, -0.55) * RotateY(180));
		obj_left_leg();

		model_view = mvstack.pop();
		mvstack.push(model_view);
		model_view *= (Translate(0.0, -1.4, 0.55) * RotateY(180));
		obj_right_leg();
	}

	void torso()
	{
		mvstack.push(model_view);

		mat4 instance = (Translate(0.0, 0.0, 0.5 * TORSO_HEIGHT) *
			Scale(TORSO_WIDTH, TORSO_WIDTH, TORSO_HEIGHT));

		glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * instance);
		glUniform4fv(draw_color, 1, color_torso);
		glDrawArrays(GL_TRIANGLES, 0, NumVertices);
		model_view = mvstack.pop();
	}

	void head()
	{
		mvstack.push(model_view);

		mat4 instance = (Translate(0.0, 0.0, 0.5 * HEAD_HEIGHT) *
			Scale(HEAD_WIDTH, HEAD_WIDTH, HEAD_HEIGHT));

		glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * instance);
		glUniform4fv(draw_color, 1, color_head);
		glDrawArrays(GL_TRIANGLES, 0, NumVertices);

		model_view = mvstack.pop();
	}

	void left_arm()
	{
		mvstack.push(model_view);

		mat4 instance = (Translate(0.0, 0.0, 0.5 * ARM_HEIGHT) *
			Scale(ARM_WIDTH,
				ARM_WIDTH,
				ARM_HEIGHT));

		glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * instance);
		glUniform4fv(draw_color, 1, color_upper_arm);
		glDrawArrays(GL_TRIANGLES, 0, NumVertices);

		model_view = mvstack.pop();
	}

	void right_arm()
	{
		mvstack.push(model_view);

		mat4 instance = (Translate(0.0, 0.0, 0.5 * ARM_HEIGHT) *
			Scale(ARM_WIDTH, ARM_WIDTH,
				ARM_HEIGHT
			));

		glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * instance);
		glUniform4fv(draw_color, 1, color_upper_arm);
		glDrawArrays(GL_TRIANGLES, 0, NumVertices);

		model_view = mvstack.pop();
	}

	void left_leg()
	{
		mvstack.push(model_view);

		mat4 instance = (Translate(0.0, 0.0, 0.5 * LEG_HEIGHT) *
			Scale(LEG_WIDTH,
				LEG_WIDTH,
				LEG_HEIGHT));

		glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * instance);
		glUniform4fv(draw_color, 1, color_upper_leg);
		glDrawArrays(GL_TRIANGLES, 0, NumVertices);

		model_view = mvstack.pop();
	}

	void right_leg()
	{
		mvstack.push(model_view);

		mat4 instance = (Translate(0.0, 0.0, 0.5 * LEG_HEIGHT) *
			Scale(LEG_WIDTH, LEG_WIDTH,
				LEG_HEIGHT
			));

		glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * instance);
		glUniform4fv(draw_color, 1, color_upper_leg);
		glDrawArrays(GL_TRIANGLES, 0, NumVertices);

		model_view = mvstack.pop();
	}

	void obj_torso()
	{
		mvstack.push(model_view);
		mp_->set_matrix(modelViewMatrix, projection, model_view, 0);
		model_view = mvstack.pop();
	}

	void obj_head()
	{
		mvstack.push(model_view);
		mp_->set_matrix(modelViewMatrix, projection, model_view, 1);
		model_view = mvstack.pop();
	}

	void obj_left_arm()
	{
		mvstack.push(model_view);

		mat4 instance = RotateZ(theta[LeftArm] + 180)*Translate(0.0, -1.5, 0.2);

		mp_->set_matrix(modelViewMatrix, projection, model_view * instance, 2);

		model_view = mvstack.pop();
	}


	void obj_right_arm()
	{
		mvstack.push(model_view);

		mat4 instance = RotateZ(theta[RightArm] + 180)*Translate(0.0, -1.5, -0.2);

		mp_->set_matrix(modelViewMatrix, projection, model_view * instance, 3);

		model_view = mvstack.pop();
	}

	void obj_left_leg()
	{
		mvstack.push(model_view);

		mat4 instance = RotateZ(-theta[LeftLeg] + 180)*Translate(0.0, -1.5, 0.0);

		mp_->set_matrix(modelViewMatrix, projection, model_view * instance, 4);

		model_view = mvstack.pop();
	}

	void obj_right_leg()
	{
		mvstack.push(model_view);

		mat4 instance = RotateZ(-theta[RightLeg] + 180)*Translate(0.0, -1.5, 0.0);

		mp_->set_matrix(modelViewMatrix, projection, model_view * instance, 5);

		model_view = mvstack.pop();
	}
};