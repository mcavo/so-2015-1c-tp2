#ifndef DRIVERS_H
#define DRIVERS_H

#include <kernel.h>

#define MAX_DRIVERS_COUNT 6
#define CONS_DRIVER 0
#define INPUT_DRIVER 1
#define PS_DRIVER 2
#define IDE_DRIVER 3

/* códigos de error */
#define EOF -1
#define NO_METHOD_EXIST -2

/* drives's struct */
typedef struct 
{
	char* name;
	int (*open_driver)(void);
	int (*read_driver)(char *buf, int size);
	int (*write_driver)(char *buf, int size);
	int (*close_driver)(void);
	int (*ioctl_driver)(void);
	int (*read_block_driver)(char *buf, int size);
	int (*write_block_driver)(char *buf, int size);
}
driver_t;

static driver_t driverTable[MAX_DRIVERS_COUNT];

#endif