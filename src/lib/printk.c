#include <kernel.h>

int 
vprintk(const char *fmt, va_list args)
{
	int i, n;
	char c;
	//char aux;
	char buf[400];
	// driver_t *cons = getDriver(CONS_DRIVER);

	n = vsprintf(buf, fmt, args);
	if ( n > 0 )
		for ( i = 0 ; i < n ; i++ )
		{
			if ( (c = buf[i]) == '\n' ) {
				mt_cons_putc('\r');
				//aux = '\r';
				//ioctl_driver_cons(CONS_PUTC,1,&aux);
				// (cons->write_driver)("\r", 1);
			}
			mt_cons_putc(c);
			//ioctl_driver_cons(CONS_PUTC,1,&c);
			// (cons->write_driver)((char*)&c, 1);
		}

	return n;
}

int 
printk(const char *fmt, ...)
{
	va_list args;
	int n;

	Atomic();
	va_start(args, fmt);
	n = vprintk(fmt, args);
	va_end(args);
	Unatomic();

	return n;
}

int 
print0(const char *fmt, ...)	// especial para interrupciones, inprime en consola 0
{
	va_list args;
	int n;

	bool ints = SetInts(false);
	//unsigned prev = mt_cons_set0();
	unsigned prev;
	//ioctl_driver_cons(CONS_SET0,1,&prev);
	mt_cons_set0(&prev);
	va_start(args, fmt);
	n = vprintk(fmt, args);
	va_end(args);
	mt_cons_setcurrent(prev);
	//ioctl_driver_cons(CONS_SETCURRENT,1,&prev);

	SetInts(ints);
	return n;
}

void
cprintk(unsigned fg, unsigned bg, const char *fmt, ...)
{
	va_list args;
	unsigned fgi, bgi;

	Atomic();
	//mt_cons_getattr(&fgi, &bgi);
	//ioctl_driver_cons(CONS_GETATTR,2,&fgi,&bgi);
	(getDriver(CONS_DRIVER)->ioctl_driver)(CONS_GETATTR,2,&fgi,&bgi);
	//mt_cons_setattr(fg, bg);
	//ioctl_driver_cons(CONS_SETATTR,2,fg,bg);
	(getDriver(CONS_DRIVER)->ioctl_driver)(CONS_SETATTR,2,fg,bg);
	va_start(args, fmt);
	vprintk(fmt, args);
	va_end(args);
	//mt_cons_setattr(fgi, bgi);
	//ioctl_driver_cons(CONS_SETATTR,2,fgi,bgi);
	(getDriver(CONS_DRIVER)->ioctl_driver)(CONS_SETATTR,2,fgi,bgi);
	Unatomic();
}

