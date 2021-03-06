//Includes de C
#include <time.h>
#include <stdio.h>

//Includes de C++
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <cmath>

//Includes de GL y Glut
#define GL_GLEXT_PROTOTYPES //para usar buffers
#include <GL/glut.h>

//Includes de libfreenect
#include <libfreenect.hpp>
#include "Mutex.cpp"
#include "MyFreenectDevice.h"

using namespace std;

//declaracion por adelantado de ciertas funciones
void generarMalla(); // crea la estructura de la malla
void actualizarMalla(); //actualiza profundidades
void regen(); //rengenera la vista de opengl
void findExtremos(uint8_t &maximo, uint8_t &minimo); //obtiene los maximos de profundidad
std::vector<float> tria2vert(int idx); //dado un indice de triangulo, devuelve las coordenadas de sus vertices

//Declara por adelantado los callbacks
void display_cb();
void Mouse_cb(int button, int state, int x, int y);
void Idle_cb();
void reshape_cb (int w, int h);

//----------------VARIABLES GLOBALES Y DEFAULTS---------------------------------
const unsigned int puntos_horiz = 640;
const unsigned int puntos_vert = 480;

unsigned int maximo, minimo;

double depth[puntos_vert][puntos_horiz];


//Valores usados para la malla
unsigned int cant_triangulos = (puntos_horiz-1)*(puntos_vert-1)*2;
static const unsigned int cant_indices = cant_triangulos*3;
//array de vertices: x1,y1,z1,x2,y2,z2,....,xn,yn,zn con n=puntos_horiz * puntos_vert
static const unsigned int cant_vertices = puntos_horiz*puntos_vert*3;

GLint * vertices = new GLint[cant_vertices];  //array de Vertices
GLubyte * indices = new GLubyte[cant_indices]; //indices de los triangulos
std::vector<unsigned int> triangulos; //valores de los vertices de cada triangulo


GLuint elementbuffer; //para guardar los valores de cada uno de los triangulos

//Declaracion de instancia de kinect
Freenect::Freenect freenect;
MyFreenectDevice& device = freenect.createDevice<MyFreenectDevice>(0);

//Variables de Estado y valores por defecto
int 
	w=640, h=480, // tamaño de la ventana
	boton = -1, // boton del mouse clickeado
	xclick,yclick; // x e y cuando clickeo un boton

float
  escala = 100, escala0, // escala de los objetos window/modelo pixeles/unidad
  eye[] = {0,0,5}, target[] = {0,0,0}, up[] = {0,1,0}, // camara, mirando hacia arriba y vertical
  znear = 0.3f, zfar = 10.f,               //clipping planes cercano y alejado de la camara (en 5 => veo de 3 a -3)
  lat=0,lat0,lon=0,lon0; // latitud y longitud (grados) de la camara (0=al clickear)
  double h0, w0;

bool // variables de estado de este programa
	rota = false;       // gira continuamente los objetos respecto de y
 
 

short modifiers = 0;  // ctrl, alt, shift (de GLUT)
static const double  PI = 4*atan(1.0), 
                     R2G = 180/PI;

inline short get_modifiers() { 
    return modifiers = (short) glutGetModifiers();
}

//----------------------FUNCIONES DE SOPORTE DE OPENGL-------------------------
//----glpclview.c-----
// Do the projection from u,v,depth to X,Y,Z directly in an opengl matrix
// These numbers come from a combination of the ros kinect_node wiki, and
// nicolas burrus' posts.
void LoadVertexMatrix() {
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

/* SIN USO
// This matrix comes from a combination of nicolas burrus's calibration post
// and some python code I haven't documented yet.
void LoadRGBMatrix() {
    float mat[16] = {
        5.34866271e+02,   3.89654806e+00,   0.00000000e+00,   1.74704200e-02,
        -4.70724694e+00,  -5.28843603e+02,   0.00000000e+00,  -1.22753400e-02,
        -3.19670762e+02,  -2.60999685e+02,   0.00000000e+00,  -9.99772000e-01,
        -6.98445586e+00,   3.31139785e+00,   0.00000000e+00,   1.09167360e-02
    };
    glMultMatrixf(mat);
}
*/

//------------------------------------------------------------------

// calcula la posicion de la camara y el vector up (al manipular o si gira)
void calc_eye() {
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

//Inicializa los callbacks y la ventana de glut
void initialize() {
	glutInitDisplayMode(GLUT_DEPTH|GLUT_RGB|GLUT_DOUBLE|GLUT_ALPHA);
	glutInitWindowSize (w,h);
	
	glutCreateWindow ("Point Cloud");
	glutDisplayFunc (display_cb);
	glutMouseFunc(Mouse_cb); // botones apretados
	glutIdleFunc(display_cb); // registra el callback
	//glutIdleFunc(Idle_cb); // registra el callback
	//glutReshapeFunc (ReSizeGLScene);
	glutReshapeFunc (reshape_cb);
	
	glClearColor(1.f, 1.f, 1.f, 1.f);
	glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL); // habilita el z-buffer
	glEnable(GL_NORMALIZE); // normaliza las normales para que el scaling no moleste
	
	//inicializacion para triangulos
	glEnableClientState(GL_VERTEX_ARRAY);
    
    glGenBuffers(1, &elementbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangulos.size() * sizeof(unsigned int), &triangulos[0], GL_STATIC_DRAW);

	regen();
}

// Si no hace nada hace esto
void Idle_cb() {
	/* Hace analisis de framerate. Sin uso
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
	*/
	glutPostRedisplay(); // redibujar
}

// Regenera la matriz de proyeccion cuando cambia algun parametro de la vista
void regen() {
	// matriz de proyeccion
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// semiancho y semialto de la ventana a escala en el target
	w0 = (double) w/2/escala;
	h0 = (double) h/2/escala;
	// frustum, perspective y ortho son respecto al eye con los z positivos

	double
		delta[3] = {(target[0]-eye[0]),
					(target[1]-eye[1]),
					(target[2]-eye[2]) },
		dist = sqrt( delta[0]*delta[0] +
					 delta[1]*delta[1] +
					 delta[2]*delta[2] );
	//w0 y h0 en el near
	w0 *= znear/dist;
	h0 *= znear/dist; 
	
	//glFrustum(0, w/2, 0, h/2, znear, zfar);
	glFrustum(-w0, w0, -h0, h0, znear, zfar);
	//Proyeccion ortogonal, sin uso
	//glOrtho(-w0, w0, -h0, h0,  znear, zfar); //glOrtho(0, w, 0, h, znear, zfar);

	// matriz del modelo
	glMatrixMode(GL_MODELVIEW); 
	glLoadIdentity(); 

	// ubica la camara
	gluLookAt(   eye[0],   eye[1],   eye[2],
			  target[0],target[1],target[2],
				  up[0],    up[1],    up[2]);
	
	//rota los objetos alrededor de y
	LoadVertexMatrix();
	
	// avisa que hay que redibujar
	glutPostRedisplay(); 
}

/* SIN USO
void ReSizeGLScene(int Width, int Height){
    glViewport(0,0,Width,Height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, 4/3., 0.3, 200);
    glMatrixMode(GL_MODELVIEW);
}*/


//Maneja cambios en la ventana
void reshape_cb (int w, int h) {
    glViewport(0,0,w,h);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0,w,0,h);
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity();
    regen();
}

// Movimientos del mouse. 
void Motion_cb(int xm, int ym){ // drag
	ym = h - ym; // y abajo
	if (boton==GLUT_LEFT_BUTTON){
		if (modifiers == GLUT_ACTIVE_SHIFT){ // cambio de escala
			escala = escala0*exp((yclick - ym)/100.0);
			regen();
		} else { // manipulacion de la camara
			// el signo menos es para que parezcan movimientos del objeto
			lon = lon0 - (xm-xclick)*180.0/h;
			if (lon>=360) 
				lon -= 360;
			
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
void Mouse_cb(int button, int state, int x, int y){
	y = h-y;
	static bool old_rota = false;
	if (button == GLUT_LEFT_BUTTON){
		if (state == GLUT_DOWN) {
			xclick = x; 
			yclick = y;
			boton = button;
			old_rota = rota; 
			rota = false;
			
			get_modifiers();
			glutMotionFunc(Motion_cb); // callback para los drags
			
			if (modifiers==GLUT_ACTIVE_SHIFT) // cambio de escala
				escala0 = escala;
			else // manipulacion
				lat0=lat; lon0=lon;
			
		}
		else if (state==GLUT_UP){
			rota = old_rota;
			boton = -1;
			glutMotionFunc(0); // anula el callback para los drags
		}
	}
}

void display_cb() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
    if(!device.getDepth(depth))
        return; //evita iteraciones si no hay nuevo frame
	glPointSize(1);

    // Index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

    // Draw the triangles !
    glDrawElements(
        GL_TRIANGLES,      // mode
        triangulos.size(),    // count
        GL_UNSIGNED_INT,   // type
        (void*)0           // element array buffer offset
    );



/*
    unsigned int idx = 0;
   
    glBegin(GL_POINTS);
    for(unsigned int i = 0; i < puntos_vert; ++i){
		for(unsigned int j = 0; j < puntos_horiz; ++j){
			//optiene coordenadas de mundo
			uint16_t newx = j;
			uint16_t newy = i;
			unsigned int zeta = (unsigned int) depth[i][j];
            
            //actualiza la malla
            vertices[idx+2] = zeta; 
            idx += 3;
	
			//interpola el color segun profundidad
			float newz = float(zeta-minimo)/(maximo-minimo); 
			
            newz = newz*255;
			
			//(sacado de un mail de la lista de kinect) nslakshmiprabha@gmail.com
			//double newz = 1.0 / (double(zeta) * -0.0030711016 + 3.3309495161); 
		
            //std::cout<<newx<<' '<<newy<<' '<<newz<<' '<<zeta<<'\n';
			//dibuja el vertex
			glColor3ub(0,newz,0); glVertex3i(newx,newy, zeta);
		}
	}
	glEnd();
   

    glColor3f(0,0,1.0);
	glBegin(GL_TRIANGLES);
	for(unsigned int i = 0; i < cant_triangulos; i++){
		std::vector<float> coords = tria2vert(i);
        //std::cout<<coords[0]<<' '<<coords[1]<<' '<<coords[2]<<'\n';
		glVertex3f(coords[0],coords[1],coords[2]);
		glVertex3f(coords[3],coords[4],coords[5]);
		glVertex3f(coords[6],coords[7],coords[8]);
	}
	glEnd();

    */
    //glutPostRedisplay(); 
    glutSwapBuffers(); 
    regen();
}




//--------------------FIN CALLBACKS---------------------------------------------

//--------------------METODOS PROPIOS-------------------------------------------

double diffclock(clock_t clock1,clock_t clock2){
	double diffticks = clock1 - clock2;
	return (diffticks*1000)/CLOCKS_PER_SEC;
}

//dado un triangulo,devuelve las coordenadas de sus 3 vertices en un std::vector
std::vector<float> tria2vert(int idx){
	std::vector<float> coords;
	
	int base = idx*3;
	int indexes []= {base, base+1,base+2}; 
	
	float p1 []= {	vertices[indices[indexes[0] ]*3 ], 
					vertices[indices[indexes[0] ]*3+1], 
					vertices[indices[indexes[0] ]*3+2]  };
	float p2 []= {	vertices[indices[indexes[1] ]*3 ], 
					vertices[indices[indexes[1] ]*3+1], 
					vertices[indices[indexes[1] ]*3+2]  };
	float p3 []= {	vertices[indices[indexes[2] ]*3 ], 
					vertices[indices[indexes[2] ]*3+1], 
					vertices[indices[indexes[2] ]*3+2]  };
	
	coords.push_back(p1[0]); coords.push_back(p1[1]); coords.push_back(p1[2]);
	coords.push_back(p2[0]); coords.push_back(p2[1]); coords.push_back(p2[2]);
	coords.push_back(p3[0]); coords.push_back(p3[1]); coords.push_back(p3[2]);
	
	return coords;	
}

//Crea la malla, definiendo los indices que corresponden a cada triangulo
void generarMalla(){
    unsigned int idx = 0;
    //guarda en el array vertices, las coordenadas de los puntos {...,x_i,y_i,z_i,...}
    for(unsigned int i = 0; i < puntos_vert; i++){
        for(unsigned int j = 0; j < puntos_horiz; j++){
			vertices[idx] = j; 
            vertices[idx+1] = i;
            vertices[idx+2] = depth[i][j];
            idx += 3;
        }
    }

    /*crea triangulos tipo 1
    //  i-----i+1
    //   \tipo |
    //    \ 1  |
    //     \   |
    //       i+puntos_horiz+1 */
    idx = 0;
    for(unsigned int i = 0; i < puntos_horiz*puntos_vert - 1 - puntos_horiz; i++){
		indices[idx] = i;
		indices[idx+1] = i+1;
		indices[idx+2] = i + puntos_horiz + 1;
		idx += 3;
		//if(i / float(puntos_horiz) >= (puntos_horiz-1)) 
		//    break; //termine de recorrer
		if(((i+2) % puntos_horiz) == 0) 
			i++; //llego al borde, me salto uno
    }
   
    /*crea triangulos tipo 2 
    //  i-puntos_horiz
    //  |  \
    //  | 2 \
    //  |tipo\
    //  i-----i+1*/

    for(unsigned int i = puntos_horiz; i < puntos_horiz*puntos_vert - 1; i++){
		indices[idx] = i - puntos_horiz;
		indices[idx+1] = i;
		indices[idx+2] = i+1;
		idx += 3;
		//if(i / puntos_horiz >= (puntos_horiz)) 
		//	break; //termine de recorrer
		if(((i+2) % puntos_horiz) == 0) 
			i++; //llego al borde, me salto uno
    }

    //genera el vector que se usara para dibujar
    for (unsigned int i = 0; i < cant_indices; i++) {
        unsigned int indice_triangulo = indices[i];
        int xi = vertices[indice_triangulo*3];
        int yi = vertices[indice_triangulo*3+1];
        int zi = vertices[indice_triangulo*3+2];
        triangulos.push_back(xi);
        triangulos.push_back(yi);
        triangulos.push_back(zi);
    }
}

//Actualiza la profundidad, que es lo unico que cambio entre iteraciones sucesivas
void actualizarMalla() {
    unsigned int idx = 0;
    for(unsigned int i = 0; i < puntos_vert; i++){
        for(unsigned int j = 0; j < puntos_horiz; j++){
            vertices[idx+2] = depth[i][j];
            idx += 3;
        }
    }
    std::cout<<"Ver que debe actualizar triangulos tambien";
}

//Encuentra las profundidades máximas y mínimas
void findExtremos(){
	maximo = 0;
	minimo = 255;
	
	for(int i = 0; i < h; i++){
		for(int j = 0; j < w; j++){
			uint8_t value = (unsigned int) depth[i][j];
			if(value > maximo)
				maximo = value;
			if(value < minimo)
				minimo = value;
		}
	}
	if(maximo == minimo)
		maximo++;
}


int main(int argc, char **argv) {
    glutInit (&argc, argv);

    std::cout<<"Inicializando Kinect\n";
    initialize();

    triangulos.resize(cant_indices);
    //device.startVideo();
    device.startDepth();
    
    device.getDepth(depth); //carga por primera vez la profundidad
    
    std::cout<<"Generacion de la malla\n";
    //Obtiene los extremos para interpolar el color segun profundidad
    findExtremos();
    generarMalla(); //crea la estructura de la malla   

    std::cout<<"Iniciando bucle principal\n";
    glutMainLoop();
    
    //device.stopVideo();
    device.stopDepth();
    return 0;
}
