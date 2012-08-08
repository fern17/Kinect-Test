#include <GL/glut.h> 

void init(void); 
void display(void); 
void reshape(int, int); 

static GLfloat vertices[] = 
{ 
-1.0, -1.0, 1.0, 
1.0, -1.0, 1.0, 
1.0, 1.0, 1.0, 
-1.0, 1.0, 1.0, 
-1.0, -1.0, -1.0, 
1.0, -1.0, -1.0, 
1.0, 1.0, -1.0, 
-1.0, 1.0, -1.0 
}; 


static GLfloat colors[] = 
{0.5,1.0,0.5, 
1.0,0.0,0.0, 
0.0,1.0,0.0, 
0.0,0.0,1.0, 
1.0,1.0,0.0, 
0.0,1.0,1.0, 
}; 

static GLubyte frontIndices[] = {4,5,6,7}; 
static GLubyte rightIndices[] = {1,2,6,5}; 
static GLubyte bottomIndices[] = {0,1,5,4}; 
static GLubyte backIndices[] = {0,3,2,1}; 
static GLubyte leftIndices[] = {0,4,7,3}; 
static GLubyte topIndices[] = {2,3,7,6}; 


int main(int argc, char** argv) 
{ 
glutInit(&argc, argv); 
glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB); 
glutInitWindowSize(500, 500); 
glutInitWindowPosition(100, 100); 
glutCreateWindow(argv[0]); 
init(); 
glutDisplayFunc(display); 
glutReshapeFunc(reshape); 
glutMainLoop(); 
return 0; 
} 

void init(void) 
{ 
glClearColor(0.0,0.0,0.0,0.0); 
glShadeModel(GL_FLAT); 
glEnableClientState(GL_VERTEX_ARRAY); 
glVertexPointer(3, GL_FLOAT, 0, vertices); 
} 

void display(void) 
{ 
glClear(GL_COLOR_BUFFER_BIT); 
glColor3f(1.0,1.0,1.0); 

glMatrixMode(GL_MODELVIEW); 
glPushMatrix(); 

glTranslated(0.5, 0.5, -0.5); 
glScaled(0.2, 0.2, 0.2); 
glRotated(30.0, 1.0, 0.0, 0.0); 
glRotated(30.0, 0.0, 1.0, 0.0); 

glColor3fv(colors+0); 
glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, frontIndices); 
glColor3fv(colors+3); 
glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, rightIndices); 
glColor3fv(colors+6); 
glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, bottomIndices); 
glColor3fv(colors+9); 
glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, backIndices); 
glColor3fv(colors+12); 
glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, leftIndices); 
glColor3fv(colors+15); 
glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, topIndices); 

glPopMatrix(); 
glFlush(); 
} 

void reshape(int w, int h) 
{ 
glViewport(0, 0, (GLsizei) w, (GLsizei) h); 
glMatrixMode(GL_PROJECTION); 
glLoadIdentity(); 
{ 
GLdouble aspect=((GLdouble)w)/((GLdouble)h); 
if(aspect>1.0) 
gluOrtho2D(0.0, aspect, 0.0, (GLdouble) 1.0); 
else 
gluOrtho2D(0.0, (GLdouble) 1.0, 0.0, (GLdouble)1.0/aspect); 
} 
} 
