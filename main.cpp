#include <assert.h>
#include <iostream>
#include <cstdlib>
#include <time.h>

#include "Angel.h"
#include "TriMesh.h"
#include "mesh.h"
#include "FreeImage.h"
#include "Mesh_Painter.h"
#include "MatrixStack.h"
#include "Sky.h"
#include "Camera.h"

int NumVertices = 36;

vec3 robot;
int actionTime = 0; 

float sensitivity = 0.3;
float movespeed = 0.75;

bool fisrtMouse = true;
float lastXpos = 500;
float lastYpos = 400;

int treeNum = 5;
std::vector<int> treeHeight;
std::vector<float> treePosition;

std::vector<My_Mesh*> my_meshs;
Mesh_Painter* mp_;

mat4 projection;
mat4 modelViewMatrix;
vec3 lightpos = (0.0, 10.0, 40.0);

MatrixStack  mvstack;
mat4         model_view;
GLuint       ModelView, Projection, ViewMatrix;
GLuint       draw_color;

Sky *sky;

Camera camera;

#define TORSO_HEIGHT 5.0
#define TORSO_WIDTH 3.0
#define ARM_HEIGHT 3.0
#define LEG_WIDTH  0.5
#define LEG_HEIGHT 3.0
#define ARM_WIDTH  0.5
#define HEAD_HEIGHT 1.5
#define HEAD_WIDTH 1.0

//----------------- robot set up --------------------------
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

void torso()
{
    mvstack.push(model_view);

    mat4 instance = (Translate( 0.0, 0.0 ,0.5 * TORSO_HEIGHT) *
		      Scale(TORSO_WIDTH, TORSO_WIDTH, TORSO_HEIGHT));
    
    glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view * instance );
    glDrawArrays( GL_TRIANGLES, 0, NumVertices );
    model_view = mvstack.pop();
}

void head()
{
	mvstack.push(model_view);

	mat4 instance = (Translate(0.0, 0.0 ,0.5 * HEAD_HEIGHT) *
		Scale(HEAD_WIDTH, HEAD_WIDTH ,HEAD_HEIGHT));

	glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * instance);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	model_view = mvstack.pop();
}

void left_arm()
{
	mvstack.push(model_view);

	mat4 instance = (Translate(0.0,0.0 , 0.5 * ARM_HEIGHT) *
		Scale(ARM_WIDTH,
			ARM_WIDTH ,
			ARM_HEIGHT));

	glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * instance);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	model_view = mvstack.pop();
}

void right_arm()
{
	mvstack.push(model_view);

	mat4 instance = (Translate(0.0, 0.0 ,0.5 * ARM_HEIGHT) *
		Scale(ARM_WIDTH, ARM_WIDTH,
			ARM_HEIGHT
			));

	glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * instance);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	model_view = mvstack.pop();
}

void left_leg()
{
	mvstack.push(model_view);

	mat4 instance = (Translate(0.0, 0.0 ,0.5 * LEG_HEIGHT) *
		Scale(LEG_WIDTH,
			LEG_WIDTH ,
			LEG_HEIGHT));

	glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * instance);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	model_view = mvstack.pop();
}

void right_leg()
{
	mvstack.push(model_view);

	mat4 instance = (Translate(0.0,0.0 ,0.5 * LEG_HEIGHT) *
		Scale(LEG_WIDTH, LEG_WIDTH,
			LEG_HEIGHT
			));

	glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * instance);
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

	mat4 instance = RotateZ(theta[LeftArm]+180)*Translate(0.0, -1.5, 0.2);

	mp_->set_matrix(modelViewMatrix, projection, model_view * instance, 2);

	model_view = mvstack.pop();
}

void obj_right_arm()
{
	mvstack.push(model_view);

	mat4 instance =RotateZ(theta[RightArm]+180)*Translate(0.0, -1.5, -0.2);

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

//-----------------------------

// change view range
double zoom = 150;
float fov = 45.0;
float aspect = 1.0;

// camera parameters
float radius = 50.0;
float rotateAngle = -90.0;
float upAngle = 20.0;

void display()
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	mp_->light[0] = lightpos[0];
	mp_->light[1] = lightpos[1];
	mp_->light[2] = lightpos[2];

	float x =  robot.x+radius*cos(upAngle*M_PI / 180.0)*sin(rotateAngle*M_PI / 180.0);
	float y =  robot.y+radius*cos(upAngle*M_PI / 180.0)*cos(rotateAngle*M_PI / 180.0);
	float z =  robot.z+radius*sin(upAngle*M_PI / 180.0);

	vec4 eye = vec4(x, y, z, 1.0);
	vec4 at = vec4(robot.x, robot.y, 15.0, 0.0);
	vec4 up = vec4(0.0, 0.0, 1.0, 0.0);

	camera.modelMatrix = mat4(1.0);
	camera.viewMatrix = camera.lookAt(eye, at, up);
	modelViewMatrix = camera.viewMatrix * camera.modelMatrix;
	glUniformMatrix4fv(ViewMatrix, 1.0, GL_TRUE, &modelViewMatrix[0][0]);


	model_view = Translate(robot.x, robot.y, robot.z + 4.3)*RotateZ(theta[Torso])*RotateX(90);
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

	model_view = mvstack.pop();
	
	// ground
	mat4 groudtranste = Translate(0.0 , 0.0 , 0.3)*RotateX(270)*Scale(500,1,500);
	mp_->set_matrix(modelViewMatrix, projection, groudtranste, 6);

	// trees
	for (int i = 0; i < treeNum; i++)
	{
		float height = treeHeight[i*2];
		int x, y;
		x = treePosition[i * 2];
		y = treePosition[i * 2 + 1];
		mp_->set_matrix(modelViewMatrix, projection, Translate(x, y, 0)*RotateX(90)*RotateY(-90)*Scale(1.5, height, 1.5), 7 + i * 4);
		mp_->set_matrix(modelViewMatrix, projection, Translate(x, y, 0)*RotateX(90)*RotateY(-90)*Scale(1.5, height, 1.5), 8 + i * 4);

		height = treeHeight[i * 2 + 1];
		x = treePosition[i * 2 + 2];
		y = treePosition[i * 2 + 3];
		mp_->set_matrix(modelViewMatrix, projection, Translate(x, y, 0)*RotateX(90)*RotateY(90)*Scale(1.5, height, 1.5), 9 + i * 4);
		mp_->set_matrix(modelViewMatrix, projection, Translate(x, y, 0)*RotateX(90)*RotateY(90)*Scale(1.5, height, 1.5), 10 + i * 4);
	}
	
	sky->setMatrix(modelViewMatrix*RotateX(90)*RotateY(90)*Scale(5, 5, 5), projection);
	sky->drawsky();

	mp_->draw_meshes();
	
    glutSwapBuffers();
}

void walk(int speed) 
{
	actionTime++;
	if (actionTime <= speed)
	{
		theta[LeftArm]  += 60.0  / speed;
		theta[RightArm] -= 60.0  / speed;
		theta[LeftLeg]  += 30.0  / speed;
		theta[RightLeg] -= 30.0  / speed;
	}
	else if (actionTime <= 3 * speed) 
	{
		theta[LeftArm] -= 60.0 / speed;
		theta[RightArm]+= 60.0 / speed;
		theta[LeftLeg] -= 30.0 / speed;
		theta[RightLeg] += 30.0 / speed;
	}
	else if (actionTime <= 4 * speed) 
	{
		theta[LeftArm] += 60.0 / speed;
		theta[RightArm] -= 60.0 / speed;
		theta[LeftLeg] += 30.0 / speed;
		theta[RightLeg] -= 30.0 / speed;
	}
	else
		actionTime = 0;
}

void init(void)
{	
	sky = new Sky();

	robot.x = 40;
	robot.y = 80;
	robot.z = 0.01;

	srand(time(NULL));
	for (int i = 0; i < treeNum * 2; i++)
	{
		int h = rand() % 20 + 8;
		float height = h * 0.1;

		if (h < 0.8)
			h = 0.8;

		std::cout << height << std::endl;

		treeHeight.push_back(height);

		int x = rand() % 80 - 20;
		int y = rand() % 80 - 20;

		treePosition.push_back(x);
		treePosition.push_back(y);
	}

	mp_ = new Mesh_Painter;
	
	// steve
	My_Mesh* body = new My_Mesh;
	body->load_obj("texture/steve-obj/body.obj");
	body->set_texture_file("texture/steve-obj/steve.png");
	mp_->add_mesh(body);
	my_meshs.push_back(body);
	
	My_Mesh* head = new My_Mesh;
	head->load_obj("texture/steve-obj/head.obj");
	head->set_texture_file("texture/steve-obj/steve.png");
	mp_->add_mesh(head);
	my_meshs.push_back(head);

	My_Mesh* larm = new My_Mesh;
	larm->load_obj("texture/steve-obj/larm.obj");
	larm->set_texture_file("texture/steve-obj/steve.png");
	mp_->add_mesh(larm);
	my_meshs.push_back(larm);

	My_Mesh* rarm = new My_Mesh;
	rarm->load_obj("texture/steve-obj/rarm.obj");
	rarm->set_texture_file("texture/steve-obj/steve.png");
	mp_->add_mesh(rarm);
	my_meshs.push_back(rarm);

	My_Mesh* lleg = new My_Mesh;
	lleg->load_obj("texture/steve-obj/lleg.obj");
	lleg->set_texture_file("texture/steve-obj/steve.png");
	mp_->add_mesh(lleg);
	my_meshs.push_back(lleg);

	My_Mesh* rleg = new My_Mesh;
	rleg->load_obj("texture/steve-obj/rleg.obj");
	rleg->set_texture_file("texture/steve-obj/steve.png");
	mp_->add_mesh(rleg);
	my_meshs.push_back(rleg);

	// other meshes
	My_Mesh* ground = new My_Mesh;
	ground->load_obj("texture/road/road.obj");
	ground->set_texture_file("texture/road/ground.jpg");
	ground->hasShadow = false;
	mp_->add_mesh(ground);
	my_meshs.push_back(ground);

	My_Mesh* trunk1 = new My_Mesh;
	trunk1->load_obj("texture/tree/tree3.1.obj");
	trunk1->set_texture_file("texture/ree/tree3.1.dds");

	My_Mesh* leaves1 = new My_Mesh;
	leaves1->load_obj("texture/tree/tree3.2.obj");
	leaves1->set_texture_file("texture/ree/tree.2.dds");

	My_Mesh* trunk2 = new My_Mesh;
	trunk2->load_obj("texture/tree/tree4.1.obj");
	trunk2->set_texture_file("texture/tree/tree.1.dds");

	My_Mesh* leaves2 = new My_Mesh;
	leaves2->load_obj("texture/tree/tree4.2.obj");
	leaves2->set_texture_file("texture/tree/tree.2.dds");

	for (int i = 0; i < treeNum; i++)
	{
		mp_->add_mesh(trunk1);
		my_meshs.push_back(trunk1);
				
		mp_->add_mesh(leaves1);
		my_meshs.push_back(leaves1);
		
		mp_->add_mesh(trunk2);
		my_meshs.push_back(trunk2);

		mp_->add_mesh(leaves2);
		my_meshs.push_back(leaves2);
	}

	mp_->init_shaders("vShader.glsl", "fShader.glsl");
	mp_->update_vertex_buffer();
	mp_->update_texture();
	
    glEnable( GL_DEPTH_TEST );
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    glClearColor( 0.53, 0.81, 0.92, 1.0 );
	std::cout << "Initialized" << std::endl;
}

void reshape(int width, int height)
{
	glViewport(0, 0, width, height);

	GLfloat left = -zoom, right = zoom;
	GLfloat bottom = -0.5*zoom, top = 1.5*zoom;
	GLfloat zNear = -zoom, zFar = zoom;

	GLfloat Aspect = GLfloat(width) / height;

	if (Aspect > 1.0)
	{
		left *= Aspect;
		right *= Aspect;
	}
	else
	{
		bottom /= Aspect;
		top /= Aspect;
	}

	projection = projection = camera.perspective(fov, aspect, 0.1, zoom);
	glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);

	model_view = mat4(1.0);   // An Identity matrix
}

void mouseCallback(int xpos, int ypos)
{
	if (fisrtMouse) {
		lastXpos = xpos;
		lastYpos = ypos;
		fisrtMouse = false;
	}

	// compute offset
	float xOffset = xpos - lastXpos;
	float yOffset = lastYpos - ypos;
	lastXpos = xpos;
	lastYpos = ypos;
	
	xOffset *= sensitivity;
	yOffset *= sensitivity;

	// update angle
	rotateAngle += xOffset;
	upAngle += yOffset;

	if (upAngle > 89.0f)
		upAngle = 89.0f;
	if (upAngle < 0.0f)
		upAngle = 0.0f;

	glutPostRedisplay();
}

void mouse(int button, int state, int x, int y)
{
	// change angle of steve's head
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		theta[angle] += 5.0;
		if (theta[angle] > 45.0) { theta[angle] = 45.0; }
	}

	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		theta[angle] -= 5.0;
		if (theta[angle] < -45.0) { theta[angle] = -45.0; }
	}

	glutPostRedisplay();
}

void keyboardCallback(unsigned char key, int x, int y)
{
	int speed = 5;
	float step = 0.5;
	float movespeed = 5;

    switch( key ) {
	case 033: // Escape Key
		exit(EXIT_SUCCESS);
		break;
	case 'w':   // move forward
		walk(speed);
		robot.x += step;
		theta[Torso] = 0;
		break;
	case 's': // move backward
		walk(speed);
		robot.x -= step;
		theta[Torso] = 180;
		break;
	case 'a': // move left
		walk(speed);
		robot.y += step;
		theta[Torso] = 90;
		break;
	case 'd':  // move right
		walk(speed);
		robot.y -= step;
		theta[Torso] = 270;
		break;

	case 'i':
		radius -= 0.1 * movespeed;
		if (radius < 15)
			radius = 15;
		break;
	case 'k':
		radius += 0.1 * movespeed;
		if (radius > 80)
			radius = 80;
		break;
	case 'j': // rotate the camera
		rotateAngle -= 1 * movespeed;
		break;
	case 'l': // rotate the camera
		rotateAngle += 1 * movespeed;
		break;

	case 'q': case 'Q':
	    exit(EXIT_SUCCESS);
	    break;
    }
	glutPostRedisplay();
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1000, 800);
	glutInitWindowPosition(50, 50);
    glutInitContextVersion(3, 2);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutCreateWindow("interactive viewer");
	glewExperimental = GL_TRUE;
    glewInit();

    init();

    glutDisplayFunc(display);

    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboardCallback);
    glutMouseFunc(mouse);
	glutPassiveMotionFunc(mouseCallback);

    glutMainLoop();
    return 0;
}
