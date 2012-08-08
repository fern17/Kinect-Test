#include <iostream>

using namespace std;

float vertices[] = {0,2,0,1,2,0,2,2,0,0,1,0,1,1,0,2,1,0,0,0,0,1,0,0,2,0,1};
unsigned int puntos_horiz = 3;
unsigned int puntos_vert = 3;
const unsigned int cant_triangulos = (puntos_horiz-1) * (puntos_vert-1) * 2;
const unsigned int size_indices = cant_triangulos * 3;
unsigned int indices[24];

void generarMalla(){
    unsigned int idx = 0;
    //crea triangulos tipo 1
    //  i-----i+1
    //   \tipo |
    //    \ 1  |
    //     \   |
    //       i+puntos_horiz
    idx = 0;
    for(unsigned int i = 0; i <= cant_triangulos/2; i++){
       indices[idx] = i;
       indices[idx+1] = i+1;
       indices[idx+2] = i + puntos_horiz + 1;
       cout<<indices[idx]<<' '<<indices[idx+1]<<' '<<indices[idx+2]<<endl;
       idx += 3;
       if(i / puntos_horiz >= (puntos_horiz-1)) 
           break; //termine de recorrer
       if(((i+2) % puntos_horiz) == 0){ 
           i++; //llego al borde, me salto uno
       }
    }
//    cout<<idx<<endl;
     //crea triangulos tipo 2 
    //  i-puntos_horiz
    //  |  \
    //  | 2 \
    //  |tipo\
    //  i-----i+1

    for(unsigned int i = puntos_horiz; i < puntos_horiz*puntos_vert - 1; i++){
       indices[idx] = i - puntos_horiz;
       indices[idx+1] = i;
       indices[idx+2] = i+1;
       cout<<indices[idx]<<' '<<indices[idx+1]<<' '<<indices[idx+2]<<endl;
       idx += 3;
       if(i / puntos_horiz >= (puntos_horiz)) 
           break; //termine de recorrer
       if(((i+2) % puntos_horiz) == 0) 
           i++; //llego al borde, me salto uno
    }
}

int main(){
    generarMalla();
}
