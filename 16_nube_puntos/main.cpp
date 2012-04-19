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


#include <GL/glut.h>

using namespace cv;
using namespace std;


void runOpenCV(); //declaracion por adelantado

//------------------------------------------------------------
// variables globales y defaults

Mat depthMat(Size(640,480),CV_16UC1);
Mat depthf  (Size(640,480),CV_8UC1);
Mat rgbMat(Size(640,480),CV_8UC3,Scalar(0));
int escalaGrande = 1000000000;
Freenect::Freenect freenect;
MyFreenectDevice& device = freenect.createDevice<MyFreenectDevice>(0);

int
  w=640, h=480, // tamaño de la ventana
  boton=-1, // boton del mouse clickeado
  xclick,yclick, // x e y cuando clickeo un boton
  plano, // 0=x 1=y 2=z
  maxZ = 1000;

float
  escala = 100, escala0, // escala de los objetos window/modelo pixeles/unidad
  eye[] = {0,0,5}, target[] = {0,0,0}, up[] = {0,1,0}, // camara, mirando hacia y vertical
  znear = 3.f, zfar = 6.f, //clipping planes cercano y alejado de la camara (en 5 => veo de 3 a -3)
  amy, amy0, // angulo del modelo alrededor del eje y
  ac0, rc0, // angulo resp x y distancia al target de la camara al clickear
  lat=0,lat0,lon=0,lon0; // latitud y longitud (grados) de la camara (0=al clickear)
  double h0, w0;
bool // variables de estado de este programa
  perspectiva=true, // perspectiva u ortogonal
  rota=false,       // gira continuamente los objetos respecto de y
  dibuja=true,      // false si esta minimizado
  wire=false,       // dibuja lineas o no
  relleno=true,     // dibuja relleno o no
  smooth=true,      // normales por nodo o por plano
  cl_info=true,     // informa por la linea de comandos
  antialias=false,  // antialiasing
  blend=false;      // transparencias

short modifiers = 0;  // ctrl, alt, shift (de GLUT)
static const double PI = 4*atan(1.0), R2G = 180/PI;

inline short get_modifiers() {return modifiers=(short)glutGetModifiers();}

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
  // rota los objetos alrededor de y
//  glRotatef(amy,0,1,0);

  glutPostRedisplay(); // avisa que hay que redibujar
}


// calcula la posicion de la camara y el vector up (al manipular o si gira)
void calc_eye(){
  static const double g2r = atan(1.0)/45;// grados a radianaes 
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

void reshape_cb (int w, int h) {
    if (w==0||h==0) return;
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
    
    glPointSize(1);
    runOpenCV();
    
    glBegin(GL_POINTS);
	for(int i = 0; i < depthf.rows; i++){
	  for(int j = 0; j < depthf.cols; j++){
        double zeta = depthMat.at<double>(i,j)*(255.0/2048.0)/escalaGrande;
        float newx = (i-w/2)/escala;
        float newy = (j-h/2)/escala;
        std::cout<<zeta<<std::endl;
        double newz = (zeta*(zfar-znear))/maxZ;
        float colorblue = newz/maxZ;
        glColor3f(0,0,colorblue);
        glVertex3f(newx, newy, newz);
	  }
	}
    glEnd();
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
    glutMouseFunc(Mouse_cb); // botones apretados
    glutReshapeFunc (reshape_cb);
    glClearColor(1.f, 1.f, 1.f, 1.f);
    glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL); // habilita el z-buffer
    regen();
}




double diffclock(clock_t clock1,clock_t clock2){
	double diffticks = clock1 - clock2;
	return (diffticks*1000)/CLOCKS_PER_SEC;
}


void runOpenCV(){
      //device.getVideo(rgbMat);
      device.getDepth(depthMat);
      //depthMat.convertTo(depthf, CV_8UC1, 255.0/2048.0);

      //cv::imshow("rgb", rgbMat);
      //cv::imshow("depth",depthf);

}


int main(int argc, char **argv) {
    glutInit (&argc, argv);

    initialize();
    
    namedWindow("rgb",CV_WINDOW_AUTOSIZE);
    //namedWindow("depth",CV_WINDOW_AUTOSIZE);
    device.startVideo();
    device.startDepth();
   
    
    glutMainLoop();
    
    device.stopVideo();
    device.stopDepth();
    return 0;
}
