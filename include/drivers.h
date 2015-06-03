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

static driver_t driverTable[MAX_DRIVERS_COUNT];

#endif