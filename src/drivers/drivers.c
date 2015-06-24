#include <kernel.h>

static driver_t driverTable[MAX_DRIVERS_COUNT];

void
mt_init_drivers(void)
{
	//driverTable[CONS_DRIVER] = *mt_cons_init();
	driver_t * consdriv = malloc(sizeof(driver_t));
	ioctl_driver_cons(CONS_INIT,1,&consdriv);
	driverTable[CONS_DRIVER]= *consdriv;

	//driverTable[INPUT_DRIVER] = *mt_input_init();
	driver_t * inputdriv = malloc(sizeof(driver_t));
	ioctl_driver_keyboard(INPUT_INIT,1,&inputdriv);
	driverTable[CONS_DRIVER]= *inputdriv;

	//driverTable[PS2_DRIVER] = *mt_ps2_init();
	driver_t * ps2driv = malloc(sizeof(driver_t));
	ioctl_driver_ps2(PS2_INIT,1,&ps2driv);
	driverTable[PS2_DRIVER] = *ps2driv;

	//driverTable[IDE_DRIVER] = *mt_ide_init();
	driver_t * idedriv = malloc(sizeof(driver_t));
	ioctl_driver_ide(IDE_INIT,1,&idedriv);
	driverTable[IDE_DRIVER] = *idedriv;
}

driver_t *getDriver(int driver)
{
	return &driverTable[driver];
}

