#include <libfreenect.hpp>
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <cmath>
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <time.h>
#include <stdio.h>
#include "Mutex.cpp"
#include "MyFreenectDevice.h"

#include "libfreenect_sync.h" 
#include <GL/glut.h>

using namespace cv;
using namespace std;


void runOpenCV(); //declaracion por adelantado
void regen();
//------------------------------------------------------------
// variables globales y defaults

Mat depthMat   (Size(640,480),CV_16UC1);
Mat depthFrame (Size(640,480),CV_8UC1);
Mat rgbMat     (Size(640,480),CV_8UC3, Scalar(0));
int escalaGrande = 1000000000; //factor por el cual divido las profundidades de Kinect
Freenect::Freenect freenect;
MyFreenectDevice& device = freenect.createDevice<MyFreenectDevice>(0);


GLuint gl_rgb_tex;

int
  w=640, h=480, // tamaño de la ventana
  boton = -1, // boton del mouse clickeado
  xclick,yclick, // x e y cuando clickeo un boton
  plano, // 0=x 1=y 2=z
  maxZ = 1000;//el maximo de profundidad para puntos a lo lejos

float
  escala = 100, escala0, // escala de los objetos window/modelo pixeles/unidad
  eye[] = {0,0,5}, target[] = {0,0,0}, up[] = {0,1,0}, // camara, mirando hacia arriba y vertical
  znear = 3.f, zfar = 6.f, //clipping planes cercano y alejado de la camara (en 5 => veo de 3 a -3)
  amy, amy0, // angulo del modelo alrededor del eje y
  ac0, rc0, // angulo resp x y distancia al target de la camara al clickear
  lat=0,lat0,lon=0,lon0; // latitud y longitud (grados) de la camara (0=al clickear)
  double h0, w0;
bool // variables de estado de este programa
  perspectiva = true, // perspectiva u ortogonal
  rota = false,       // gira continuamente los objetos respecto de y
  dibuja = true,      // false si esta minimizado
  animado = false,    // el objeto actual es animado
  wire = false,       // dibuja lineas o no
  relleno = true,     // dibuja relleno o no
  smooth = true,      // normales por nodo o por plano
  cl_info = true,     // informa por la linea de comandos
  antialias = false,  // antialiasing
  blend = false;      // transparencias

short modifiers = 0;  // ctrl, alt, shift (de GLUT)
static const double PI = 4*atan(1.0), R2G = 180/PI;

inline short get_modifiers() {return modifiers=(short)glutGetModifiers();}
// temporizador:
static const int ms_lista[]={1,2,5,10,20,50,100,200,500,1000,2000,5000},ms_n=12;
static int ms_i=4,msecs=ms_lista[ms_i]; // milisegundos por frame

//-----------------glpclview.c-------------------------------------------
// Do the projection from u,v,depth to X,Y,Z directly in an opengl matrix
// These numbers come from a combination of the ros kinect_node wiki, and
// nicolas burrus' posts.
void LoadVertexMatrix()
{
    float fx = 594.21f;
    float fy = 591.04f;
    float a = -0.0030711f;
    float b = 3.3309495f;
    float cx = 339.5f;
    float cy = 242.7f;
    GLfloat mat[16] = {
        1/fx,     0,  0, 0,
        0,    -1/fy,  0, 0,
        0,       0,  0, a,
        -cx/fx, cy/fy, -1, b
    };
    glMultMatrixf(mat);
}


// This matrix comes from a combination of nicolas burrus's calibration post
// and some python code I haven't documented yet.
void LoadRGBMatrix()
{
    float mat[16] = {
        5.34866271e+02,   3.89654806e+00,   0.00000000e+00,   1.74704200e-02,
        -4.70724694e+00,  -5.28843603e+02,   0.00000000e+00,  -1.22753400e-02,
        -3.19670762e+02,  -2.60999685e+02,   0.00000000e+00,  -9.99772000e-01,
        -6.98445586e+00,   3.31139785e+00,   0.00000000e+00,   1.09167360e-02
    };
    glMultMatrixf(mat);
}


//------------------------------------------------------------------

// calcula la posicion de la camara y el vector up (al manipular o si gira)
void calc_eye(){
  static const double g2r = atan(1.0)/45;// grados a radianes 
  double 
    latr = lat*g2r,
    lonr = lon*g2r,
    slat = sin(latr),
    clat = cos(latr),
    slon = sin(lonr),
    clon = cos(lonr);

  up[1] = clat; 
  up[0] = -slat*slon; 
  up[2] = -slat*clon;

  eye[1] = 5*slat; 
  eye[0] = 5*clat*slon; 
  eye[2] = 5*clat*clon;
  regen();
}

// Si no hace nada hace esto
void Idle_cb() {
  static int suma;
  static int counter = 0;				// esto es para analisis del framerate real
  
  // Cuenta el lapso de tiempo
  static int anterior = glutGet(GLUT_ELAPSED_TIME); 	// milisegundos desde que arranco
  if (msecs != 1){ 					// si msecs es 1 no pierdo tiempo
    int tiempo = glutGet(GLUT_ELAPSED_TIME);
    int lapso = tiempo - anterior;
    
    if (lapso < msecs) return;
    
    suma += lapso;
    if (++counter == 100) {
      //std::cout << "<ms/frame>= " << suma/100.0 << std::endl;
      counter = suma = 0;
    }
    anterior = tiempo;
  }
  if (rota) { // lo camara gira 1 grado alrededor de la vertical
    if (--lon < 0) lon=359;
    calc_eye();
  }
  else glutPostRedisplay(); // redibujar
}

// Regenera la matriz de proyeccion
// cuando cambia algun parametro de la vista
void regen() {
//  if (cl_info) cout << "regen" << endl;
  if (!dibuja) return;
  // matriz de proyeccion
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  // semiancho y semialto de la ventana a escala en el target
  w0 = (double) w/2/escala;
  h0 = (double) h/2/escala;
  // frustum, perspective y ortho son respecto al eye con los z positivos
  if (perspectiva){ // perspectiva
    double
      delta[3] = {(target[0]-eye[0]),
                (target[1]-eye[1]),
                (target[2]-eye[2])},
      dist = sqrt(delta[0]*delta[0]+
                delta[1]*delta[1]+
                delta[2]*delta[2]);
      w0 *= znear/dist,
      h0 *= znear/dist; //w0 y h0 en el near
    //glFrustum(0, w/2, 0, h/2, znear, zfar);
    glFrustum(-w0, w0, -h0, h0, znear, zfar);
  }
  else { // proyeccion ortogonal
    glOrtho(-w0, w0, -h0, h0, znear, zfar);
  }

  glMatrixMode(GL_MODELVIEW); glLoadIdentity(); // matriz del modelo

  gluLookAt(   eye[0],   eye[1],   eye[2],
              target[0],target[1],target[2],
                  up[0],    up[1],    up[2]);// ubica la camara
  //rota los objetos alrededor de y
  //glRotatef(amy,0,1,0);
  LoadVertexMatrix();
  
  glutPostRedisplay(); // avisa que hay que redibujar
}


void ReSizeGLScene(int Width, int Height)
{
    glViewport(0,0,Width,Height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, 4/3., 0.3, 200);
    glMatrixMode(GL_MODELVIEW);
}


void reshape_cb (int w, int h) {
    if (w==0||h==0){ //minimiza
	dibuja=false; // no dibuja mas
	glutIdleFunc(0); // no llama a cada rato a esa funcion
	return;
    }
    else if (!dibuja && w && h){// des-minimiza
	dibuja=true; // ahora si dibuja
	if (animado || rota) 
	    glutIdleFunc(Idle_cb); // registra de nuevo el callback
    }
    glViewport(0,0,w,h);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    gluOrtho2D(0,w,0,h);
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();
    regen();
}

void display_cb() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    short *depth = 0;
    char *rgb = 0;
    uint32_t ts;
    glPointSize(1);
//    runOpenCV();
    if (freenect_sync_get_depth((void**)&depth, &ts, 0, FREENECT_DEPTH_11BIT) < 0)
        std::cout<<"no_kinect_quit()"<<std::endl;
    if (freenect_sync_get_video((void**)&rgb, &ts, 0, FREENECT_VIDEO_RGB) < 0)
        std::cout<<"no_kinect_quit()"<<std::endl;
    std::cout<<"passed\n";
    glScalef(1/640.0f,1/480.0f,1);
    LoadRGBMatrix();
    LoadVertexMatrix();
    glMatrixMode(GL_MODELVIEW);
    
    static unsigned int indices[480][640];
    static short xyz[480][640][3];
    int i,j;
    for (i = 0; i < 480; i++) {
        for (j = 0; j < 640; j++) {
            xyz[i][j][0] = j;
            xyz[i][j][1] = i;
            xyz[i][j][2] = depth[i*640+j];
            indices[i][j] = i*640+j;
        }
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glPushMatrix();
    //glScalef(zoom,zoom,1);
    glTranslatef(0,0,-3.5);
    //glRotatef(rotangles[0], 1,0,0);
    //glRotatef(rotangles[1], 0,1,0);
    glTranslatef(0,0,1.5);

    LoadVertexMatrix();

    // Set the projection from the XYZ to the texture image
    /*glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glScalef(1/640.0f,1/480.0f,1);
    LoadRGBMatrix();
    LoadVertexMatrix();
    glMatrixMode(GL_MODELVIEW);*/

    glPointSize(1);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_SHORT, 0, xyz);
    //glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    //glTexCoordPointer(3, GL_SHORT, 0, xyz);

    /*if (color)
        glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, gl_rgb_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, rgb);
*/
    glPointSize(2.0f);
    glDrawElements(GL_POINTS, 640*480, GL_UNSIGNED_INT, indices);
    glPopMatrix();
    //glDisable(GL_TEXTURE_2D);
    glutSwapBuffers();


/*
    glBegin(GL_POINTS);
	for(int i = 0; i < 480; i++){
	  for(int j = 0; j < 640; j++){
        double newx = j;
        double newy = i;
        double zeta = depth[j*640+i];
        double newz = 1.0 / (double(zeta) * -0.0030711016 + 3.3309495161); //(sacado de un mail de la lista de kinect) nslakshmiprabha@gmail.com
        std::cout<<zeta<<' '<<newz<<std::endl;
        if (zeta > 1) 
            zeta = zeta/double(escalaGrande);
        //double newx = (i-w/2)/escala;
        //double newy = (j-h/2)/escala;
        
//        double newz = (zeta*(zfar-znear))/maxZ;
        
        
        float colorblue = newz/maxZ;
        glColor3f(0,0,colorblue);
        glVertex3f(newx,newy, newz);
        //glVertex3f(newx, newy, newz);
	  }
	}
    glEnd();
    */
    glutSwapBuffers();
}


//------------------------------------------------------------
// Movimientos del mouse
void Motion_cb(int xm, int ym){ // drag
  ym=h-ym; // y abajo
  if (boton==GLUT_LEFT_BUTTON){
    if (modifiers==GLUT_ACTIVE_SHIFT){ // cambio de escala
      escala=escala0*exp((yclick-ym)/100.0);
      regen();
    }
    else { // manipulacion de la camara
      // el signo menos es para que parezcan movimientos del objeto
      lon = lon0 - (xm-xclick)*180.0/h;
      if (lon>=360) 
          lon-=360;
      lat = lat0 - (ym-yclick)*180.0/w; 
      if (lat>=90) 
          lat=89.99;
      if (lat<=-90) 
          lat=-89.99;
      calc_eye();
    }
  }
}


// Clicks del mouse
// GLUT LEFT BUTTON, GLUT MIDDLE BUTTON, or GLUT RIGHT BUTTON
// The state parameter is either GLUT UP or GLUT DOWN
// glutGetModifiers may be called to determine the state of modifier keys
void Mouse_cb(int button, int state, int x, int y){
  y = h-y;
  static bool old_rota=false;
  if (button == GLUT_LEFT_BUTTON){
    if (state == GLUT_DOWN) {
      xclick = x; 
      yclick = y;
      boton = button;
      old_rota = rota; 
      rota = false;
      get_modifiers();
      glutMotionFunc(Motion_cb); // callback para los drags
      if (modifiers==GLUT_ACTIVE_SHIFT){ // cambio de escala
        escala0 = escala;
      }
      else { // manipulacion
        lat0=lat; lon0=lon;
      }
    }
    else if (state==GLUT_UP){
      rota = old_rota;
      boton = -1;
      glutMotionFunc(0); // anula el callback para los drags
    }
  }
}

void initialize() {
    glutInitDisplayMode(GLUT_DEPTH|GLUT_RGB|GLUT_DOUBLE|GLUT_ALPHA);
    glutInitWindowSize (w,h);
    glutInitWindowPosition (100,100);
    glutCreateWindow ("Profundidades");
    glutDisplayFunc (display_cb);
    //glutDisplayFunc (DrawGLScene);
    glutMouseFunc(Mouse_cb); // botones apretados
    //glutReshapeFunc (reshape_cb);
    glutReshapeFunc (ReSizeGLScene);
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL); // habilita el z-buffer
    glEnable(GL_NORMALIZE); // normaliza las normales para que el scaling no moleste
    if ( !(dibuja && (animado | rota))) 
        glutIdleFunc(0); // no llama a cada rato a esa funcion
    else 
        glutIdleFunc(Idle_cb); // registra el callback
    regen();

}


double diffclock(clock_t clock1,clock_t clock2){
	double diffticks = clock1 - clock2;
	return (diffticks*1000)/CLOCKS_PER_SEC;
}


void runOpenCV(){
      device.getVideo(rgbMat);
      device.getDepth(depthMat);
      depthMat.convertTo(depthFrame, CV_8UC1, 255.0/2048.0);

      cv::imshow("rgb", rgbMat);
      cv::imshow("depth",depthFrame);
}


int main(int argc, char **argv) {
    glutInit (&argc, argv);

    initialize();
    
    namedWindow("rgb",CV_WINDOW_AUTOSIZE);
    namedWindow("depth",CV_WINDOW_AUTOSIZE);
    device.startVideo();
    device.startDepth();
   
    glutMainLoop();
    
    device.stopVideo();
    device.stopDepth();
    return 0;
}
