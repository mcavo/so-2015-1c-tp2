#ifndef DRIVERS_H
#define DRIVERS_H

#include <kernel.h>


#define MAX_DRIVERS_COUNT 6
#define CONS_DRIVER 0
#define INPUT_DRIVER 1
#define PS2_DRIVER 2
#define IDE_DRIVER 3

/* códigos de error */

#define ERR_DRIVER_ERROR 		-1
#define ERR_NO_METHOD_EXIST 	-2
#define ERR_INVALID_ARGUMENT	-3

/* drives's struct */
typedef struct 
{
	char* name;
	int (*open_driver)(void);
	int (*read_driver)(char *buf, unsigned size);
	int (*write_driver)(char *buf, unsigned size);
	int (*close_driver)(void);
	int (*ioctl_driver)(int type,int minor, ...);
	int (*read_block_driver)(unsigned minor, unsigned block, unsigned nblocks, void *buffer);
	int (*write_block_driver)(unsigned minor, unsigned block, unsigned nblocks, void *buffer);
}
driver_t;


driver_t *getDriver(int driver);

#endif
