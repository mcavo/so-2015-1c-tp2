#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#define MODEL 'm'
#define CAPACITY 'c'
#define CLEAN_BUFFER while(getchar()!='\n')





// // Devuelve el modelo del disco, o NULL si no está presente
// char *
// mt_ide_model(unsigned minor)
// {	

// 	char* ret ="Este es el model";
//     return ret;
// }
// // Devuelve la capacidad del disco en sectores, o 0 si no está presente
// unsigned
// mt_ide_capacity(unsigned minor)
// {
//     return 12;
// }


/*
ioctl para IDE

Parametros:
Recibe un int type (MODEL o CAPACITY) y un unsigned minor.

Retorno segun type:
MODEL: La funcion ioctl con este parametro retorna un char* con el modelo del disco, o NULL si no esta presente.
CAPACITY: La funcion ioctl con este parametro retorna un unsigned con la capacidad del disco en sectores, o 0 si no esta presente.

IMPORTANTE: Tener el cuidado de tratar el retorno de forma pertinente. Ej: Desreferenciar el retorno de la funcion al llamarla en modo CAPACITY.

*/
// char *ioctl(int type, ...) {

//     va_list pa;
//     char *ret;
// 	unsigned aux;

//     va_start(pa, type);

//         switch(type){
//             case MODEL:
//                  ret = mt_ide_model(va_arg(pa, unsigned));
// 				//printf("%s\n",ret);
//                 break;
//             case CAPACITY:
// 				aux = mt_ide_capacity(va_arg(pa, unsigned));	
//                 ret = (char*)(&aux);

//                 break;
            
//             }
    
        
//     va_end(pa);
// if(type==CAPACITY)
// printf("%d\n",*ret);
//     return ret;
// }


// void main(char* args){

// 	int m = MODEL;
// 	int c = CAPACITY;

//     char * modelo = malloc(sizeof(char)*50);
//     char * capacity = malloc(sizeof(int)*50);
	
// 	modelo = ioctl('m',0);

// 	capacity = ioctl(c,0);


//     printf("%s\n", modelo);
//     printf("%d\n", (unsigned)(*capacity));



// }
