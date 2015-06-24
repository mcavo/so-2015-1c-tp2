#include <kernel.h>
#define IDE 1
#define CONS 2
#define EXIT 3
#define CLEAN_BUFFER while(getchar()!='\n');

static int showMenu();
static void cons_tester();
static void ide_tester();
static void test_ide(unsigned minor);


int
interface_tester_main(int argc, char *argv[])
{
	while(1){
		int option = showMenu();
	
		switch(option){
			case IDE:
			printk("Eligio ide\n");
			ide_tester();
			break;
			case CONS:
			printk("Eligio consola\n");
			cons_tester();
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


static void ide_tester(){

	unsigned i;

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
