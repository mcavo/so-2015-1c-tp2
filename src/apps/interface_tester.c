#include <kernel.h>
#define IDE 1
#define CONS 2
#define EXIT 3
//#define CLEAN_BUFFER for(int i=0;i<10;i++){getch();}

static int showMenu();
static void cons_tester();
static void ide_tester();
static void test_ide(unsigned minor);


int
interface_tester_main(int argc, char *argv[])
{
	int option=showMenu();

	while((option = showMenu())!=EXIT){

	
		switch(option){

			case IDE:
				ide_tester();

			break;

			case CONS:
				cons_tester();
			break;
			
			default:
				cprintk(RED, BLACK,"Opcion incorrecta.\n\n");
			break;

		}
	}

	return 0;
}


static void ide_tester(){

	unsigned i;
	
	driver_t *cons = getDriver(CONS_DRIVER);

	cons->ioctl_driver(CONS_CLEAR,0);
	
		cprintk(LIGHTMAGENTA, BLACK,"\n \
	** TEST DISCO RIGIDO **\n\
	\n\n");

	for ( i = IDE_PRI_MASTER  ; i <= IDE_SEC_SLAVE ; i++ )
		test_ide(i);

}

static void test_ide(unsigned minor){

	driver_t *disc = getDriver(IDE_DRIVER);
	

	printk("Disco %u:\n", minor);

	char* model;
	disc->ioctl_driver(IDE_MODEL,2,minor,&model);

	if (model==NULL)
	{
		printk("    No detectado\n");
		return;
	}

	unsigned capacity;
	disc->ioctl_driver(IDE_CAPACITY,2,minor,&capacity);
	if ( capacity )
	{
		printk("    Tipo ATA, modelo %s\n", model);
		printk("    Capacidad %u sectores (%u MB)\n", capacity, capacity / ((1024 * 1024) / SECTOR_SIZE));
	}
	else
	{
		printk("    Tipo ATAPI, modelo %s\n", model);
		return;
	}

}

static void cons_tester(){
//Puedo hacer gotoxy cursor y en el medio de la pantalla escribir cosas tipo "Esto es una prueba de driver de consola" con mt_cons_puts

	driver_t *cons = getDriver(CONS_DRIVER);

	cons->ioctl_driver(CONS_CLEAR,0);

}

static int showMenu(){
	int choice = -1,i;

	cprintk(MAGENTA, BLACK,"\n \
	** TESTER DE INTERFAZ DE DRIVERS **\n\
	\n\n");
	printk("	1.Testear Driver Disco Rigido\n\
	2.Testear Driver Consola\n\
	3.Salir\n\n\
	Seleccione que driver desea testear:\n\
	");
//	scanf("%d", &choice);
//	CLEAN_BUFFER;
	choice =getch()-48;

	return choice;
}
