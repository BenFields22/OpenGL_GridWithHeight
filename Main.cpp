#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <vector>

#include <iostream>

#define _USE_MATH_DEFINES
#include <math.h>

#ifdef _WIN32
#include <windows.h>
#pragma warning(disable:4996)
#include <GL/glew.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <GL/glslprogram.h>

#include <glm/glm.hpp>
#include <glm/gtc\matrix_transform.hpp>
#include <glm/gtc\type_ptr.hpp>

enum {
	TOP,
	BOTTOM
};

int NUMNODES = 10;

#define XMIN	 0.
#define XMAX	 3.
#define YMIN	 0.
#define YMAX	 3.

#define TOPZ00  0.
#define TOPZ10  1.
#define TOPZ20  0.
#define TOPZ30  0.

#define TOPZ01  1.
#define TOPZ11  6.
#define TOPZ21  1.
#define TOPZ31  0.

#define TOPZ02  0.
#define TOPZ12  1.
#define TOPZ22  0.
#define TOPZ32  4.

#define TOPZ03  3.
#define TOPZ13  2.
#define TOPZ23  3.
#define TOPZ33  3.

#define BOTZ00  0.
#define BOTZ10  -3.
#define BOTZ20  0.
#define BOTZ30  0.

#define BOTZ01  -2.
#define BOTZ11  10.
#define BOTZ21  -2.
#define BOTZ31  0.

#define BOTZ02  0.
#define BOTZ12  -5.
#define BOTZ22  0.
#define BOTZ32  -6.

#define BOTZ03  -3.
#define BOTZ13   2.
#define BOTZ23  -8.
#define BOTZ33  -3.




// the escape key:

#define ESCAPE		0x1b

int MainWindow;
GLSLProgram  *Shader1;
float Time, red = 0.0;
GLuint VBO, VAO, EBO,VAO2,VBO2;
int valid1;
float xangle=0.0f;
float yangle=0.0f;

const int MS_PER_CYCLE = 16;

void	Animate();
void	Display();
void	InitGraphics();
void	Resize(int, int);
void	Keyboard(unsigned char c, int x, int y);
void assignHeights(std::vector<GLfloat>& myPos, int m, int n, int TorB);

std::vector<GLfloat> createGrid(float w,float d,int m,int n, int TorB)
{
	float dx = w / float(n-1);
	float dz= d / float(m-1);
	float halfWidth = 0.5f*w;
	float halfDepth = 0.5f*d;

	std::vector<GLfloat> myVertices;
	for (int i = 0; i < m; i++)
	{
		float z = halfDepth - i*dz;
		for (int j = 0; j < n; j++)
		{
			float x = -halfWidth + j*dx;
			myVertices.push_back(x);
			myVertices.push_back(0.0f);
			myVertices.push_back(z);
		}
	}

	
	/*int point = 1;
	for (int i = 0; i < 3*m*n; i+=3)
	{
		std::cout << "Point: " << point << ": [" << myVertices[i] << "," << myVertices[i + 1] << "," << myVertices[i + 2] << "]" << std::endl;
		point++;
	}*/
	assignHeights(myVertices, m, n,TorB);
	/*point = 1;
	for (int i = 0; i < 3 * m*n; i += 3)
	{
		std::cout << "Point: " << point << ": [" << myVertices[i] << "," << myVertices[i + 1] << "," << myVertices[i + 2] << "]" << std::endl;
		point++;
	}*/
	return myVertices;
}

std::vector<int> createIndices(int m, int n)
{
	int numIndices = (m - 1)*(n - 1) * 2*3;
	std::vector<int> myIndices;
	for (int i = 0; i < m-1; i++)
	{
		for (int j = 0; j < n-1; j++)
		{
			myIndices.push_back(i*n+j);
			myIndices.push_back(i*n+j+1);
			myIndices.push_back((i+1)*n+j);

			myIndices.push_back((i+1)*n + j);
			myIndices.push_back(i*n + j + 1);
			myIndices.push_back((i + 1)*n + j+1);
		}
	}
	
	return myIndices;
}


GLfloat
TopHeight(int iu, int iv)	// iu,iv = 0 .. NUMNODES-1
{
	float u = (float)iu / (float)(NUMNODES - 1);
	float v = (float)iv / (float)(NUMNODES - 1);

	// the basis functions:

	float bu0 = (1. - u) * (1. - u) * (1. - u);
	float bu1 = 3. * u * (1. - u) * (1. - u);
	float bu2 = 3. * u * u * (1. - u);
	float bu3 = u * u * u;

	float bv0 = (1. - v) * (1. - v) * (1. - v);
	float bv1 = 3. * v * (1. - v) * (1. - v);
	float bv2 = 3. * v * v * (1. - v);
	float bv3 = v * v * v;

	// finally, we get to compute something:


	float top = bu0 * (bv0*TOPZ00 + bv1*TOPZ01 + bv2*TOPZ02 + bv3*TOPZ03)
		+ bu1 * (bv0*TOPZ10 + bv1*TOPZ11 + bv2*TOPZ12 + bv3*TOPZ13)
		+ bu2 * (bv0*TOPZ20 + bv1*TOPZ21 + bv2*TOPZ22 + bv3*TOPZ23)
		+ bu3 * (bv0*TOPZ30 + bv1*TOPZ31 + bv2*TOPZ32 + bv3*TOPZ33);

	return (GLfloat)top;
}

GLfloat
BottomHeight(int iu, int iv)	// iu,iv = 0 .. NUMNODES-1
{
	float u = (float)iu / (float)(NUMNODES - 1);
	float v = (float)iv / (float)(NUMNODES - 1);

	// the basis functions:

	float bu0 = (1. - u) * (1. - u) * (1. - u);
	float bu1 = 3. * u * (1. - u) * (1. - u);
	float bu2 = 3. * u * u * (1. - u);
	float bu3 = u * u * u;

	float bv0 = (1. - v) * (1. - v) * (1. - v);
	float bv1 = 3. * v * (1. - v) * (1. - v);
	float bv2 = 3. * v * v * (1. - v);
	float bv3 = v * v * v;

	// finally, we get to compute something:


	float bot = bu0 * (bv0*BOTZ00 + bv1*BOTZ01 + bv2*BOTZ02 + bv3*BOTZ03)
		+ bu1 * (bv0*BOTZ10 + bv1*BOTZ11 + bv2*BOTZ12 + bv3*BOTZ13)
		+ bu2 * (bv0*BOTZ20 + bv1*BOTZ21 + bv2*BOTZ22 + bv3*BOTZ23)
		+ bu3 * (bv0*BOTZ30 + bv1*BOTZ31 + bv2*BOTZ32 + bv3*BOTZ33);

	return (GLfloat)bot;
}


void assignHeights(std::vector<GLfloat>& myPos, int m, int n, int TorP)
{
	int k = 1;
	for (int i = 0; i < m*n; i++)
	{
		int iu = i % NUMNODES;
		int iv = i / NUMNODES;
		if (TorP == TOP)
		{
			myPos[k] = TopHeight(iu, iv);
		}
		if (TorP == BOTTOM)
		{
			myPos[k] = BottomHeight(iu, iv);
		}
		
		k += 3;
	}
}


// main program:

int
main(int argc, char *argv[])
{
	// turn on the glut package:
	// (do this before checking argc and argv since it might
	// pull some command line arguments out)

	glutInit(&argc, argv);

	// setup all the graphics stuff:

	InitGraphics();

	glutSetWindow(MainWindow);
	glutMainLoop();

	// this is here to make the compiler happy:

	return 0;
}




// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display( ) from here -- let glutMainLoop( ) do it

void
Animate()
{
	// put animation stuff in here -- change some global variables
	// for Display( ) to find:

	int ms = glutGet(GLUT_ELAPSED_TIME);
	ms %= MS_PER_CYCLE;
	Time += (float)ms / (float)MS_PER_CYCLE;

	////////////////////////////////////////////////////////////////////MODELVIEWPROJECTION MATRIX/////////////////////////////////////////////////////////////////////////////////

	xangle += 1.0f;
	glm::mat4 model;
	//model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(xangle), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, glm::radians(yangle), glm::vec3(1.0f, 0.0f, 0.0f));
	//model = glm::scale(model, glm::vec3(5.0f,5.0f,5.0f));
	glm::mat4 view;
	// Note that we're translating the scene in the reverse direction of where we want to move
	view = glm::lookAt(glm::vec3(5.0f,5.0f,5.0f),glm::vec3(0.0f,0.0f,0.0f),glm::vec3(0.0f,1.0f,0.0f));
	//view = glm::rotate(view, xangle, glm::vec3(0.0f, 1.0f, 0.0f));
	//view = glm::rotate(view, yangle, glm::vec3(1.0f, 0.0f, 0.0f));

	glm::mat4 projection;
	projection = glm::perspective(glm::radians(90.0f), (float)520 / 520, 0.1f, 100.0f);
	//projection = glm::ortho(-10, 10, -10, 10);
	GLint modelLoc = glGetUniformLocation(valid1, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	GLint viewLoc = glGetUniformLocation(valid1, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	GLint projectionLoc = glGetUniformLocation(valid1, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	////////////////////////////////////////////////////////////////////MODELVIEWPROJECTION MATRIX/////////////////////////////////////////////////////////////////////////////////


	// force a call to Display( ) next time it is convenient:

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


// draw the complete scene:

void
Display()
{
	

	glutSetWindow(MainWindow);
	Shader1->Use();
	// erase the background:
	
	glDrawBuffer(GL_BACK);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

	// specify shading to be smooth:

	glShadeModel(GL_SMOOTH);


	// set the viewport to a square centered in the window:

	GLsizei vx = glutGet(GLUT_WINDOW_WIDTH);
	GLsizei vy = glutGet(GLUT_WINDOW_HEIGHT);
	GLsizei v = vx < vy ? vx : vy;			// minimum dimension
	GLint xl = (vx - v) / 2;
	GLint yb = (vy - v) / 2;
	glViewport(xl, yb, v, v);



	////////////////////////////////////////////////////////////////////DRAW CALLS/////////////////////////////////////////////////////////////////////////////////

	glBindVertexArray(VAO);

	// bind index buffer if you want to render indexed data
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	GLint colorLoc = glGetUniformLocation(valid1, "ourColor");
	glUniform3f(colorLoc, 1.0f, 0.0f, 0.0f);

	// indexed draw call
	glDrawElements(GL_TRIANGLES, ((NUMNODES-1)*(NUMNODES-1)*6), GL_UNSIGNED_INT, NULL);

	glBindVertexArray(VAO2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	colorLoc = glGetUniformLocation(valid1, "ourColor");
	glUniform3f(colorLoc, 0.0f, 0.0f, 1.0f);
	glDrawElements(GL_TRIANGLES, ((NUMNODES - 1)*(NUMNODES - 1) * 6), GL_UNSIGNED_INT, NULL);

	// swap the double-buffered framebuffers:
	glutSwapBuffers();

	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !

	glFlush();

	////////////////////////////////////////////////////////////////////DRAW CALLS/////////////////////////////////////////////////////////////////////////////////
}

// initialize the glut and OpenGL libraries:
//	also setup display lists and callback functions

void
InitGraphics()
{
	// request the display modes:
	// ask for red-green-blue-alpha color, double-buffering, and z-buffering:

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

	// set the initial window configuration:

	glutInitWindowPosition(0, 0);
	glutInitWindowSize(520, 520);

	// open the window and set its title:

	MainWindow = glutCreateWindow("SurfaceVis");
	glutSetWindowTitle("SurfaceVis");

	// set the framebuffer clear values:

	glClearColor(0., 0., 0., 0.);

	glutSetWindow(MainWindow);
	glutDisplayFunc(Display);
	glutReshapeFunc(Resize);
	glutKeyboardFunc(Keyboard);
	glutIdleFunc(Animate);


	// init glew (a window must be open to do this):

#ifdef WIN32
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		fprintf(stderr, "glewInit Error\n");
	}
	else
		fprintf(stderr, "GLEW initialized OK\n");
	fprintf(stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif


	// create shader 1
	Shader1 = new GLSLProgram();
	valid1 = Shader1->Create("vertex.vert", "fragment.frag");
	if (!valid1) {
		fprintf(stderr, "Shader cannot be created!\n");
	}
	else {
		fprintf(stderr, "Shader created.\n");
	}
	Shader1->SetVerbose(false);
	
	std::vector<GLfloat> TOPvertices = createGrid(3, 3, NUMNODES, NUMNODES,TOP);
	std::vector<GLfloat> BOTvertices = createGrid(3, 3, NUMNODES, NUMNODES, BOTTOM);
	std::vector<int> indices = createIndices(NUMNODES, NUMNODES);
	std::cout << "NODES: " << NUMNODES << std::endl;
	std::cout << "Size of vertices is: " << TOPvertices.size() << std::endl;
	std::cout << "Size of Indices is: " << indices.size() << std::endl;

	glGenVertexArrays(1, &VAO);
	glGenVertexArrays(1, &VAO2);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &VBO2);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, TOPvertices.size()*sizeof(GLfloat), TOPvertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);
	
	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0); // Unbind VAO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


	glBindVertexArray(VAO2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, BOTvertices.size() * sizeof(GLfloat), BOTvertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0); // Unbind VAO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

void
Resize(int width, int height)
{

	// don't really need to do anything since window size is
	// checked each time in Display( ):

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void
Keyboard(unsigned char c, int x, int y)
{
	switch (c)
	{
	case 'a':
		xangle -= 1.0f;
		break;		 
	case 'd':		 
		xangle += 1.0f;
		break;		 
	case 'w':		 
		yangle += 1.0f;
		break;		 
	case 's':		 
		yangle -= 1.0f;
		break;

	case ESCAPE:
		exit(0);
		break;				// happy compiler

	default:
		fprintf(stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c);
	}

	// force a call to Display( ):

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}