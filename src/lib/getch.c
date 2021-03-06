#include <kernel.h>

int 
getch(void)
{
	driver_t *input_driver = getDriver(INPUT_DRIVER);
	char c;
	return (input_driver->read_driver)(&c, 1) ? c : EOF;
}
// 
int 
getch_cond(void)
{
	unsigned char c;
	bool b;
	(getDriver(INPUT_DRIVER)->ioctl_driver)(KBD_GETCH_COND,2,&c,&b);
	return b ? c : EOF;
}

int 
getch_timed(unsigned timeout)
{
	unsigned char c;
	bool b;
	(getDriver(INPUT_DRIVER)->ioctl_driver)(KBD_GETCH_TIMED,3,&c,timeout,&b);
	return b ? c : EOF;
}
