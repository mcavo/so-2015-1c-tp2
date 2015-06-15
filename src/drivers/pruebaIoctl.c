/*#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define MODEL 'm'
#define CAPACITY 'c'
#define CLEAN_BUFFER while(getchar()!='\n')



// Devuelve el modelo del disco, o NULL si no está presente
int
mt_ide_model(char* res, unsigned minor)
{	
	strcpy(res, "Este es el model");

    return 0;
}
// Devuelve la capacidad del disco en sectores, o 0 si no está presente
int
mt_ide_capacity(int* res, unsigned minor)
{
	*res = 12;
    return 0;

}
*/

/*
ioctl para IDE

Parametros:
Recibe un int type (MODEL o CAPACITY) y un unsigned minor.

Retorno segun type:
MODEL: La funcion ioctl con este parametro retorna un char* con el modelo del disco, o NULL si no esta presente.
CAPACITY: La funcion ioctl con este parametro retorna un unsigned con la capacidad del disco en sectores, o 0 si no esta presente.

IMPORTANTE: Tener el cuidado de tratar el retorno de forma pertinente. Ej: Desreferenciar el retorno de la funcion al llamarla en modo CAPACITY.

*/
/*int ioctl(int type,int minor, ...) {

    va_list pa;


    va_start(pa, minor);

        switch(type){
            case MODEL:
		{
			//lo q devuelve va como puntero adentro de la funcion mt. tengo que cambiar su signature. En general ioctl devuelve cero si esta tdo ok.
			char *buf = va_arg(pa, char *);
			mt_ide_model(buf,minor);
		}
		
                break;
            case CAPACITY:
		{
			int *res= va_arg(pa, int *);
			mt_ide_capacity(res,minor);	
		}

                break;
            
            }
    
        
    return 0;
}


void main(char* args){

	char modelo[1000]; 
	int capacity;
	
	ioctl(MODEL,0,modelo);

	ioctl(CAPACITY,0, &capacity);

    printf("%s\n", modelo);

    printf("%d\n", capacity);

}
*/
