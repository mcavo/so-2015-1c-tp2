#include <kernel.h>

int
events_main(int argc, char *argv[])
{
	input_event_t event;
	int x, y;

	printk("ESC para salir\n");

	// Consumir eventos previos acumulados
	bool gc = true;
	while ( gc ) {
		(getDriver(INPUT_DRIVER)->ioctl_driver)(INPUT_GET_COND,2,&event,&gc);
	}

	while ( true )
	{
		(getDriver(INPUT_DRIVER)->ioctl_driver)(INPUT_GET,2,&event,NULL);
		switch ( event.type )
		{
			case KBD_EVENT:
				if ( event.kbd.scan_codes[0] == 0xE1 )
					printk("Teclado\tE1 %02.2X %02.2X\n", event.kbd.scan_codes[1], event.kbd.scan_codes[2]);
				else if ( event.kbd.scan_codes[0] == 0xE0 )
					printk("Teclado\tE0 %02.2X\n", event.kbd.scan_codes[1]);
				else
					printk("Teclado\t%02.2X\n", event.kbd.scan_codes[0]);
				if ( event.kbd.scan_codes[0] == 0x81 ) // break ESC
				{
					// Consumir teclas que no se han leído
					while ( getch_cond() != EOF )
						;					
					return 0;
				}
				break;
			case MOUSE_EVENT:
				x = event.mouse.x;
				if ( event.mouse.x_sign )
					x -= 256;
				y = event.mouse.y;
				if ( event.mouse.y_sign )
					y -= 256;
				printk("Mouse\t%s%s%s%5d%5d%5d\n", event.mouse.left_button ? "L" : "-",
					event.mouse.middle_button ? "M" : "-", event.mouse.right_button ? "R" : "-", x, y, event.mouse.z);
				break;
			default:
				break;
		}
	}

	return 0;
}
