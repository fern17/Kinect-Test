#include <vector>

#include <GL/glut.h>
#include <iostream>
//----------------VARIABLES GLOBALES Y DEFAULTS---------------------------------
const unsigned int puntos_horiz = 3;
const unsigned int puntos_vert = 3;

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


//Crea la malla, definiendo los indices que corresponden a cada triangulo
void generarMalla(){
    unsigned int idx = 0;
    //guarda en el array vertices, las coordenadas de los puntos {...,x_i,y_i,z_i,...}
    std::cout<<"Asignar vertices\n";
    for(unsigned int i = 0; i < puntos_vert; i++){
        for(unsigned int j = 0; j < puntos_horiz; j++){
			vertices[idx] = j; 
            vertices[idx+1] = i;
            vertices[idx+2] = depth[i][j];
            idx += 3;
        }
    }

    std::cout<<"Crear triangulos tipo 1\n";
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
        std::cout<<idx<<':'<<(unsigned int) indices[idx]<<' '<<(unsigned int) indices[idx+1]<<' '<<(unsigned int) indices[idx+2]<<'\n';
		idx += 3;
		//if(i / float(puntos_horiz) >= (puntos_horiz-1)) 
		//    break; //termine de recorrer
		if(((i+2) % puntos_horiz) == 0) 
			i++; //llego al borde, me salto uno
    }
   
    std::cout<<"Crear triangulos tipo 2\n";
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
        std::cout<<idx<<':'<<(unsigned int) indices[idx]<<' '<<(unsigned int) indices[idx+1]<<' '<<(unsigned int) indices[idx+2]<<'\n';
		idx += 3;
		//if(i / puntos_horiz >= (puntos_horiz)) 
		//	break; //termine de recorrer
		if(((i+2) % puntos_horiz) == 0) 
			i++; //llego al borde, me salto uno
    }

    std::cout<<"Crear vector de triangulos\n";

    int salto = 0;
    //genera el vector que se usara para dibujar
    for (unsigned int i = 0; i < cant_indices; i++, salto++) {
        std::cerr<<(unsigned int) vertices[indices[i] ]<<' ';        
        triangulos.push_back(vertices[indices[i] ]);
        
        if (salto % 3 == 0) std::cout<<'\n';
    }

}

int main(){
    for(unsigned int i = 0; i < puntos_vert; i++){
        for(unsigned int j = 0; j < puntos_horiz; j++){
            depth[i][j] = rand() % 1000+1000;
        }
     }
    std::cout<<"generar malla\n";
    generarMalla();
}
