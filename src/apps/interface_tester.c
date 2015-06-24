#include <kernel.h>
#define IDE 1
#define CONS 2
#define EXIT 3
#define CLEAN_BUFFER while(getchar()!='\n');

static int showMenu();


int
interface_tester_main(int argc, char *argv[])
{
	while(1){
		int option = showMenu();
	
		switch(option){
			case IDE:
			printk("Eligio ide\n");
			break;
			case CONS:
			printk("Eligio consola\n");
			break;
			case EXIT:
				Exit(0);
			break;
			default:
			printk("Elija una opcion correcta: \n");
			break;

		}
	}

	return 0;
}

static int showMenu(){
	int choice = -1;
	printk("\n \
	** TESTER DE INTERFAZ DE DRIVERS **\n\
	\n\n\
	1.Testear Driver Disco Rigido\n\
	2.Testear Driver Consola\n\
	3.Salir\n\n\
	Seleccione que driver desea testear:\n\
	");
//	scanf("%d", &choice);
//	CLEAN_BUFFER;
	choice =getch();
	return choice-48;
}
