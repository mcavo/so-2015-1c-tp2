#include <kernel.h>

struct Pipe_t
{
	char *			name;
	Monitor_t *		monitor;
	Condition_t *	cond_get;
	Condition_t *	cond_put;
	unsigned		size;
	unsigned		avail;
	char *			buf;
	char *			head;
	char *			tail;
	char *			end;
};

/*
--------------------------------------------------------------------------------
CreatePipe, DeletePipe - creacion y destruccion de pipes.

El parametro size determina el tamano del buffer interno. En principio puede
usarse cualquier valor; cuanto mas grande sea el buffer, mayor sera el
desacoplamiento entre las tarea que escriben y las que leen en el pipe.
--------------------------------------------------------------------------------
*/

Pipe_t *
CreatePipe(const char *name, unsigned size)
{
	char buf[200];
	Pipe_t *p = Malloc(sizeof(Pipe_t));

	p->head = p->tail = p->buf = Malloc(p->size = size);
	p->end = p->buf + size;
	p->monitor = CreateMonitor(name);
	p->name = GetName(p->monitor);
	sprintf(buf, "get %s", name);
	p->cond_get = CreateCondition(buf, p->monitor);
	sprintf(buf, "put %s", name);
	p->cond_put = CreateCondition(buf, p->monitor);

	return p;
}

void
DeletePipe(Pipe_t *p)
{
	DeleteCondition(p->cond_get);
	DeleteCondition(p->cond_put);
	DeleteMonitor(p->monitor);
	Free(p->buf);
	Free(p);
}

/*
--------------------------------------------------------------------------------
GetPipe, GetPipeCond, GetPipeTimed - lectura de un pipe.

GetPipe intenta leer del pipe, bloqueándose si el pipe está vacío.
Retorna cuando puede leer algo, aunque sea una cantidad menor a la solicitada.
GetPipeTimed se comporta igual, pero puede salir prematuramente por timeout.
GetPipeCond lee los bytes que puede y retorna inmediatamente.

Estas funciones retornan la cantidad de bytes leidos, que nunca excederá size 
o el tamaño del pipe (lo que sea menor), o eventualmente cero si no se leyó nada.
--------------------------------------------------------------------------------
*/

unsigned
GetPipe(Pipe_t *p, void *data, unsigned size)
{
	return GetPipeTimed(p, data, size, FOREVER);
}

unsigned
GetPipeCond(Pipe_t *p, void *data, unsigned size)
{
	return GetPipeTimed(p, data, size, 0);
}

unsigned
GetPipeTimed(Pipe_t *p, void *data, unsigned size, unsigned msecs)
{
	unsigned i, nbytes;
	char *d;

	if ( !size || !EnterMonitor(p->monitor) )
		return 0;

	// Si hay un timeout finito, calcular deadline.
	Time_t deadline = (msecs && msecs != FOREVER) ? Time() + msecs : 0;

	// Bloquearse si el pipe está vacío
	while ( !p->avail ) 
	{
		// Desistir si es condicional, si no esperar
		if ( !msecs || !WaitConditionTimed(p->cond_get, msecs) )
		{
			LeaveMonitor(p->monitor);
			return 0;
		}
		// Si hay que seguir esperando con deadline, recalcular timeout
		if ( deadline && !p->avail )
		{
			Time_t now = Time();
			msecs = now < deadline ? deadline - now : 0;
		}
	}

	// Leer lo que se pueda
	for ( nbytes = min(size, p->avail), d = data, i = 0 ; i < nbytes ; i++ )
	{
		*d++ = *p->head++;
		if ( p->head == p->end )
			p->head = p->buf;
	}

	// Despertar eventuales escritores bloqueados
	BroadcastCondition(p->cond_put);
	p->avail -= nbytes;

	// Retornar cantidad de bytes leídos
	LeaveMonitor(p->monitor);
	return nbytes;
}

/*
--------------------------------------------------------------------------------
PutPipe, PutPipeCond, PutPipeTimed - escritura en un pipe.

PutPipe intenta escribir en el pipe, bloqueándose hasta que haya lugar.
PutPipeTimed puede salir por timeout esperando que haya lugar.
PutPipeCond retorna inmediatamente si no hay lugar.

Estas funciones retornan la cantidad de bytes escritos: size o el tamaño del pipe
(lo que sea menor) o eventualmente cero si no se escribió nada.
Las escrituras en el pipe son atómicas. 
--------------------------------------------------------------------------------
*/

unsigned
PutPipe(Pipe_t *p, void *data, unsigned size)
{
	return PutPipeTimed(p, data, size, FOREVER);
}

unsigned
PutPipeCond(Pipe_t *p, void *data, unsigned size)
{
	return PutPipeTimed(p, data, size, 0);
}

unsigned
PutPipeTimed(Pipe_t *p, void *data, unsigned size, unsigned msecs)
{
	unsigned nbytes;
	char *d;

	if ( !size || !EnterMonitor(p->monitor) )
		return 0;

	// No se pueden escribir más bytes que el tamaño del pipe
	if ( size > p->size )
		size = p->size;

	// Si hay un timeout finito, calcular deadline.
	Time_t deadline = (msecs && msecs != FOREVER) ? Time() + msecs : 0;

	// Para que la escritura sea atómica, esperar a que haya lugar para escribir todos los bytes
	while ( (p->size - p->avail) < size ) 
	{
		// Desistir si es condicional, si no esperar
		if ( !msecs || !WaitConditionTimed(p->cond_put, msecs) )
		{
			LeaveMonitor(p->monitor);
			return 0;
		}
		// Si hay que seguir esperando con deadline, recalcular timeout
		if ( deadline && (p->size - p->avail) < size )
		{
			Time_t now = Time();
			msecs = now < deadline ? deadline - now : 0;
		}
	}

	// Hay por lo menos size bytes libres, escribir
	nbytes = size;
	d = data;
	while ( size-- )
	{
		*p->tail++ = *d++;
		if ( p->tail == p->end )
			p->tail = p->buf;
	}

	// Despertar eventuales lectores bloqueados
	if ( !p->avail )
		BroadcastCondition(p->cond_get);
	p->avail += nbytes;

	// Retornar cantidad de bytes escritos
	LeaveMonitor(p->monitor);
	return nbytes;
}

/*
--------------------------------------------------------------------------------
AvailPipe - indica la cantidad de bytes almacenada en el pipe.
--------------------------------------------------------------------------------
*/

unsigned
AvailPipe(Pipe_t *p)
{
	return p->avail;
}
