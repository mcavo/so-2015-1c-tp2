#include <kernel.h>

typedef char sector[SECTOR_SIZE];

static void
check_disk(unsigned minor)
{
	driver_t *disc = getDriver(IDE_DRIVER);

	printk("Disco %u:\n", minor);

	char* model;
	(disc->ioctl_driver)(IDE_MODEL,2,minor,&model);
	if ( !model )
	{
		printk("    No detectado\n");
		return;
	}

	unsigned capacity;
	(disc->ioctl_driver)(IDE_CAPACITY,2,minor,&capacity);
	if ( capacity )
	{
		printk("    Tipo ATA, modelo %s\n", model);
		printk("    Capacidad %u sectores (%u MB)\n", capacity, capacity / ((1024 * 1024) / SECTOR_SIZE));
	}
	else
	{
		printk("    Tipo ATAPI, modelo %s\n", model);
		return;
	}

	Time_t start;
	unsigned maxm, n, elapsed;
	sector *sectors = Malloc(128 * SECTOR_SIZE);
	maxm = (unsigned)(disc->read_block_driver)(minor, 0, 128, sectors);
	// maxm = mt_ide_read(minor, 0, 128, sectors);

	if ( !maxm ) 
	{
		printk("    Error de lectura de sectores\n");
		return;
	}

	printk("    Soporta lectura/escritura multiple de %u sectores\n", maxm);
	start = Time();
	for ( n = 0 ; n < (1024 * 1024) / (maxm * SECTOR_SIZE) ; n++ ) 
		maxm = (unsigned)(disc->write_block_driver)(minor, 0, maxm, sectors);
	elapsed = Time() - start;
	printk("    Tiempo de escritura de 1 MB: %u milisegundos\n", elapsed);

	Free(sectors);
}

int 
disk_main(int argc, char *argv[])
{
	unsigned i;

	for ( i = IDE_PRI_MASTER  ; i <= IDE_SEC_SLAVE ; i++ )
		check_disk(i);

	return 0;
}