#include <kernel.h>

static driver_t driverTable[MAX_DRIVERS_COUNT];

void
mt_init_drivers(void)
{
	driverTable[CONS_DRIVER] = *mt_cons_init();
	driverTable[INPUT_DRIVER] = *mt_input_init();
	driverTable[PS2_DRIVER] = *mt_ps2_init();
	driverTable[IDE_DRIVER] = *mt_ide_init();
}

