#include <kernel.h>
#define IDE 1
#define CONS 2
#define EXIT 3
#define TURQUESA 1
#define VIOLETA 2
#define AMARILLO 3
#define AZUL 4
#define ROJO 5


static int showMenu();
static void cons_tester();
static void ide_tester();
static void test_ide(unsigned minor);
static void show_test_cons(int color);


int
interface_tester_main(int argc, char *argv[])
{
	driver_t *cons = getDriver(CONS_DRIVER);
	cons->ioctl_driver(CONS_CLEAR,0);

	int option;

	while((option = showMenu())!=EXIT){

	
		switch(option){

			case IDE:
				cons->ioctl_driver(CONS_CLEAR,0);
				cprintk(MAGENTA, BLACK,"\n ** TEST DISCO RIGIDO **\n\n\n");
				ide_tester();

			break;

			case CONS:
				cons->ioctl_driver(CONS_CLEAR,0);
				cprintk(MAGENTA, BLACK,"\n ** TEST CONSOLA **\n\n\n");
				cons_tester();
			break;
			
			default:
				cons->ioctl_driver(CONS_CLEAR,0);
				cprintk(RED, BLACK,"Opcion incorrecta.\n\n");
			break;

		}
	}

	cons->ioctl_driver(CONS_CLEAR,0);

	return 0;
}


static void ide_tester(){

	unsigned i;
	driver_t *cons = getDriver(CONS_DRIVER);
	
	cprintk(YELLOW, BLACK, "Presione q para salir\n\n");		
	for ( i = IDE_PRI_MASTER  ; i <= IDE_SEC_SLAVE ; i++ )
		test_ide(i);

	while(getch() !='q');
	cons->ioctl_driver(CONS_CLEAR,0);	

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

	driver_t *cons = getDriver(CONS_DRIVER);
	int color,opcion;	
	show_test_cons(WHITE);

	while((opcion = getch()) !='q'){
			
		
		switch(opcion-48){
			case TURQUESA:
				color=CYAN;
			break;
			case VIOLETA:
				color=MAGENTA;				
			break;
			case AMARILLO:
				color=YELLOW;
			break;
			case AZUL:
				color=BLUE;
			break;
			case ROJO:
				color=RED;
			break;

		}
		
		show_test_cons(color);
				

	}
	
	cons->ioctl_driver(CONS_CLEAR,0);	


}

static void show_test_cons(int color){
	
	driver_t *cons = getDriver(CONS_DRIVER);
	cons->ioctl_driver(CONS_CLEAR,0);

	cprintk(MAGENTA, BLACK,"\n ** TEST CONSOLA **\n\n\n");
	cprintk(YELLOW, BLACK, "Presione q para salir\n\n");		
	
	cprintk(LIGHTMAGENTA, BLACK,"\n \
	** Seleccione un color **\n\
	\n\n");
	printk("	1.Turquesa\n\
	2.Violeta\n\
	3.Amarillo\n\
	4.Azul\n\
	5.Rojo\n\n");

	cons->ioctl_driver(CONS_GOTOXY,2,20,20);
	cprintk(color, BLACK, "ESTO ES UN TEST DE CONSOLA");
}

static int showMenu(){
	int choice = -1;

	cprintk(MAGENTA, BLACK,"\n \
	** TESTER DE INTERFAZ DE DRIVERS **\n\
	\n\n");
	printk("	1.Testear Driver Disco Rigido\n\
	2.Testear Driver Consola\n\
	3.Salir\n\n\
	Seleccione que driver desea testear:\n\
	");

	choice =getch()-'0';

	return choice;
}
