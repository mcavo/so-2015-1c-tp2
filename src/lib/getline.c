#include <kernel.h>

#define BS		0x08
#define ESC		0x1b
#define ERASEBACK "\x08 \x08"

static int 
getc(void)
{
	int c;

	while ( (c = getch()) == EOF )
		;
	if ( c != ESC )
		return c;
	while ( (c = getch()) == EOF )
		;
	if ( c != '[' )
		return c;
	while ( (c = getch()) == EOF )
		;
	return -c;
}

int
getline(char *buf, unsigned size)
{
	driver_t *cons = getDriver(CONS_DRIVER);
	char *p = buf + strlen(buf), *end = buf + size - 1;
	int c;
	unsigned xi, yi, si, ni;
	
	//mt_cons_getxy(&xi, &yi);
	//ioctl_driver_cons(CONS_GETXY,2,&xi, &yi);
	(cons->ioctl_driver)(CONS_GETXY,2,&xi, &yi);
	//si = mt_cons_nscrolls();
	//ioctl_driver_cons(CONS_NSCROLLS,1,&si);
	(cons->ioctl_driver)(CONS_NSCROLLS,1,&si);
	// mt_cons_puts(buf);
	//ioctl_driver_cons(CONS_PUTS,1,buf);
	(cons->ioctl_driver)(CONS_PUTS,1,buf);
	(cons->write_driver)(buf, 0);
	//mt_cons_clreol();
	//ioctl_driver_cons(CONS_CLREOL,0);
	(cons->ioctl_driver)(CONS_CLREOL,0);
	while (p < end)
		switch (c = getc())
		{
			case BACK:
			case FWD:
			case FIRST:
			case LAST:
				//mt_cons_gotoxy(xi, yi - (mt_cons_nscrolls() - si));
				//ioctl_driver_cons(CONS_NSCROLLS,1,&ni);
				(cons->ioctl_driver)(CONS_NSCROLLS,1,&ni);
				//ioctl_driver_cons(CONS_GOTOXY,2,xi,yi-(ni-si));
				(cons->ioctl_driver)(CONS_GOTOXY,2,xi,yi-(ni-si));
				//mt_cons_clreom();
				//ioctl_driver_cons(CONS_CLREOM,0);
				(cons->ioctl_driver)(CONS_CLREOM,0);
				*p = 0;
				return c;

			case BS:
				if (p == buf)
					break;
				if (*--p == '\t')
				{
					//mt_cons_gotoxy(xi, yi - (mt_cons_nscrolls() - si));
					//ioctl_driver_cons(CONS_NSCROLLS,1,&ni);
					(cons->ioctl_driver)(CONS_NSCROLLS,1,&ni);
					//ioctl_driver_cons(CONS_GOTOXY,2,xi,yi-(ni-si));
					(cons->ioctl_driver)(CONS_GOTOXY,2,xi,yi-(ni-si));
					//mt_cons_clreom();
					//ioctl_driver_cons(CONS_CLREOM,0);
					(cons->ioctl_driver)(CONS_CLREOM,0);
					*p = 0;
					// mt_cons_puts(buf);
					(cons->write_driver)(buf, 0);
				}
				else
					// mt_cons_puts(ERASEBACK);
					(cons->write_driver)(ERASEBACK, 0);
				break;

			case '\r':
			case '\n':
				// mt_cons_puts("\r\n");
				(cons->write_driver)("\r\n", 0);
				*p++ = '\n';
				*p = 0;
				return p - buf;

			case EOF:
				*p = 0;
				return p - buf;

			default:
				*p++ = c;
				//mt_cons_putc(c);
				//ioctl_driver_cons(CONS_PUTC,1,&c);
				(cons->ioctl_driver)(CONS_PUTC,1,&c);
				// (cons->write_driver)((char*)&c, 1);
				break;
		}

	// mt_cons_puts("<EOB>\r\n");
	(cons->write_driver)("<EOB>\r\n", 0);
	*p = 0;
	return p - buf;
}
