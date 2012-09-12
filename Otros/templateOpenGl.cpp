#include <iostream>
#include <string>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>

int g_argc;
char **g_argv;

unsigned int WIDTH = 1200;
unsigned int HEIGHT = 500;
std::string TITLE = "Window";
int window = 0;

void DrawGLScene() {
	
}

void IdleGLScene() {
	
}

void keyPressed(unsigned char key, int x, int y) {
	
}


void ReSizeGLScene(int _width, int _height) {
	
}


void InitGL(int _width, int _height) {
	
}


void glThread(){
	glutInit(&g_argc, g_argv);

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitWindowPosition(0, 0);

	window = glutCreateWindow(TITLE.c_str());

	glutDisplayFunc(&DrawGLScene);
	glutIdleFunc(&IdleGLScene);
	glutReshapeFunc(&ReSizeGLScene);
	glutKeyboardFunc(&keyPressed);

	InitGL(WIDTH, HEIGHT);

	glutMainLoop();
}

int main() {
	glThread();
	return 0;
}