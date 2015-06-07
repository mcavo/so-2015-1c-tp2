#include <kernel.h>

int 
vprintk(const char *fmt, va_list args)
{
	int i, n;
	char c;
	char buf[400];
	// driver_t *cons = getDriver(CONS_DRIVER);

	n = vsprintf(buf, fmt, args);
	if ( n > 0 )
		for ( i = 0 ; i < n ; i++ )
		{
			if ( (c = buf[i]) == '\n' )
				mt_cons_putc('\r');
				//(cons->write_driver)((unsigned char *)"\r", 0);
			mt_cons_putc(c);
			/*char *str;
			sprintf(str, "%c", c);
			(cons->write_driver)((unsigned char *)str, 0);*/
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
	unsigned prev = mt_cons_set0();
	va_start(args, fmt);
	n = vprintk(fmt, args);
	va_end(args);
	mt_cons_setcurrent(prev);
	SetInts(ints);

	return n;
}

void
cprintk(unsigned fg, unsigned bg, const char *fmt, ...)
{
	va_list args;
	unsigned fgi, bgi;

	Atomic();
	mt_cons_getattr(&fgi, &bgi);
	mt_cons_setattr(fg, bg);
	va_start(args, fmt);
	vprintk(fmt, args);
	va_end(args);
	mt_cons_setattr(fgi, bgi);
	Unatomic();
}

