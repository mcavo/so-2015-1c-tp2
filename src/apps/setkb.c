#include <kernel.h>

int
setkb_main(int argc, char **argv)
{
	driver_t *ps2 = getDriver(PS2_DRIVER);
	if ( argc > 2 )
	{
		cprintk(LIGHTRED, BLACK, "Cantidad de argumentos incorrecta\n");
		return 1;
	}
	if ( argc == 1 )
	{
		const char * kname;
		const char ** knames;
		(ps2->ioctl_driver)(PS2_GETLAYOUT,1,&kname);
		(ps2->ioctl_driver)(PS2_LAYOUTS,1,&knames);
		printk("Teclado actual: %s\n", kname);
		printk( "Disponibles:\n");
		const char **p = knames;
		while ( *p )
			printk("\t%s\n", *p++);
		return 0;
	}
	bool valid=false;
	(ps2->ioctl_driver)(PS2_SETLAYOUT,2,argv[1],&valid);
	if ( valid )
	{
		printk("Teclado actual: %s\n", argv[1]);
		return 0;
	}
	cprintk(LIGHTRED, BLACK, "Teclado %s desconocido\n", argv[1]);
	return 2;
}
