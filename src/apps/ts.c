#include <kernel.h>

static const char *title  = 
	"   Tarea Nombre       Consola  Prioridad Estado    Esperando             Timeout";

static const char *foot  = 
	"       ENTER, ESPACIO, BS: scroll      ESC: reset scroll      S: salir          ";

static char *
name(void *p)
{
	char *s = GetName(p);
	return s ? s : "";
}

static bool
getuser(unsigned *skip)
{
	int c;

	switch ( (c  = getch_timed(200)) )
	{
		case 'S':
		case 's':
			return false;
		case ' ':
		case '\r':
			++*skip;
			return true;
		case 0x08:		// BS
			if ( *skip )
				--*skip;
			return true;
		case 0x1B:		// ESC
			*skip = 0;
			return true;
	}
	return true;
}

int
ts_main(int argc, char *argv[])
{
	unsigned i, ntasks, n, skip;
	TaskInfo_t *ti, *info;
	bool enabled[NVCONS];
	bool check_cons = false;				// por defecto habilitar todas las consolas
	bool cursor;
	(getDriver(CONS_DRIVER)->ioctl_driver)(CONS_CURSOR,2,false,&cursor);

	memset(enabled, 0, sizeof enabled);
	for ( i = 1 ; i < argc ; i++ )			// habilitar las consolas especificadas
	{
		unsigned cons = atoi(argv[i]);
		if ( cons < NVCONS )
			enabled[cons] = check_cons = true;
	}

	(getDriver(CONS_DRIVER)->ioctl_driver)(CONS_CLEAR,0);
	cprintk(WHITE, BLUE, "%s", title);
	(getDriver(CONS_DRIVER)->ioctl_driver)(CONS_GOTOXY,2,0,24);
	cprintk(WHITE, BLUE, "%s", foot);
	skip = 0;
	do
	{
		info = GetTasks(&ntasks);
		for ( n = 0, i = 1, ti = info ; i < 24 && ntasks-- ; ti++ )
		{
			if ( (check_cons && !enabled[ti->consnum]) || n++ < skip )
				continue;
			(getDriver(CONS_DRIVER)->ioctl_driver)(CONS_GOTOXY,2,0,i++);
			char namebuf[20];
			sprintf(namebuf, ti->protected ? "[%.16s]" : "%.18s", name(ti->task));
			cprintk(WHITE, BLACK, "%8x %-18s %u %10u %-9s %-18.18s", ti->task, namebuf, 
				ti->consnum, ti->priority, statename(ti->state), name(ti->waiting));
			if ( ti->is_timeout )
				cprintk(WHITE, BLACK, " %10u", ti->timeout);
			else
				(getDriver(CONS_DRIVER)->ioctl_driver)(CONS_CLREOL,0);
		}
		while ( i < 24 )
		{
			(getDriver(CONS_DRIVER)->ioctl_driver)(CONS_GOTOXY,2,0,i++);
			(getDriver(CONS_DRIVER)->ioctl_driver)(CONS_CLREOL,0);
		}
		Free(info);
	}
	while ( getuser(&skip) );
	(getDriver(CONS_DRIVER)->ioctl_driver)(CONS_CLEAR,0);
	(getDriver(CONS_DRIVER)->ioctl_driver)(CONS_CURSOR,2,cursor,NULL);
	return 0;
}