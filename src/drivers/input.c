#include <kernel.h>

#define INPUTSIZE 32
#define NAME_KEYBOARD "Keyboard"

static MsgQueue_t *events[NVCONS];
static MsgQueue_t *keys[NVCONS];
static MsgQueue_t *input_focus, *input_current, *key_focus, *key_current;
static unsigned focus, current;
static driver_t *generateDriver_keyboard();

driver_t*
mt_input_init()
{
	unsigned i;
	char buf [50];

	for ( i = 0 ; i < NVCONS ; i++ )
	{
		sprintf(buf, "input events %u", i);
		events[i] = CreateMsgQueue(buf, INPUTSIZE, sizeof(input_event_t));
		sprintf(buf, "input keys %u", i);
		keys[i] = CreateMsgQueue(buf, INPUTSIZE, 1);
	}
	input_current = input_focus = events[0];
	key_current = key_focus = keys[0];
	return generateDriver_keyboard();
}

void 
mt_input_put(input_event_t *ev,bool *b)
{
	if (b!=NULL)
		*b = PutMsgQueueCond(input_focus, ev);
	else
		PutMsgQueueCond(input_focus, ev);
}

void 
mt_input_get(input_event_t *ev, bool *b)
{
	if (b!=NULL)
		*b = GetMsgQueue(input_current, ev);
	else
		GetMsgQueue(input_current, ev);
}

void 
mt_input_get_cond(input_event_t *ev, bool *b)
{
	if (b!=NULL)
		*b = GetMsgQueueCond(input_current, ev);
	else
		GetMsgQueueCond(input_current, ev);
}

void
mt_input_get_timed(input_event_t *ev, unsigned timeout, bool *b)
{
	if (b!=NULL)
		*b = GetMsgQueueTimed(input_current, ev, timeout);
	else
		GetMsgQueueTimed(input_current, ev, timeout);
}

void
mt_kbd_putch(char c, bool *b)
{
	if (b!=NULL)
		*b = PutMsgQueueCond(key_focus, &c);
	else
		PutMsgQueueCond(key_focus, &c);
}

void
mt_kbd_puts(char *s, unsigned len, bool *b)	
{
	Atomic();
	if ( INPUTSIZE - AvailMsgQueue(key_focus) < len )
	{
		Unatomic();
		if (b!=NULL)
			*b = false;
		return;
	}
	while ( len-- )
		PutMsgQueueCond(key_focus, s++);
	Unatomic();
	if (b!=NULL)
		*b = true;
}

void 
mt_kbd_getch(char *c, bool *b)
{
	if (b!=NULL)
		*b = GetMsgQueue(key_current, c);
	else
		GetMsgQueue(key_current, c);
}

void
mt_kbd_getch_cond(unsigned char *c, bool *b)
{
	if (b!=NULL)
		*b = GetMsgQueueCond(key_current, c);
	else
		GetMsgQueueCond(key_current, c);
}

void
mt_kbd_getch_timed(unsigned char *c, unsigned timeout, bool *b)
{
	if (b!=NULL)
		*b = GetMsgQueueTimed(key_current, c, timeout);
	else
		GetMsgQueueTimed(key_current, c, timeout);
}

void 
mt_input_setfocus(unsigned consnum)			// No se llama desde interrupciones
{
	Atomic();
	if ( consnum != focus )
	{
		focus = consnum;
		input_focus = events[focus];
		key_focus = keys[focus];
		//mt_cons_setfocus(focus);
		//ioctl_driver_cons(CONS_SETFOCUS,1,focus);
		(getDriver(CONS_DRIVER)->ioctl_driver)(CONS_SETFOCUS,1,focus);
	}
	Unatomic();
}

void 
mt_input_setcurrent(unsigned consnum)		// Se llama con interrupciones deshabilitadas
{
	if ( consnum == current )
		return;
	current = consnum;
	input_current = events[current];
	key_current = keys[current];
	//mt_cons_setcurrent(current);
	//ioctl_driver_cons(CONS_SETCURRENT,1,current);
	(getDriver(CONS_DRIVER)->ioctl_driver)(CONS_SETCURRENT,1,current);
} 


/* driver interface */

int open_driver_keyboard(void){
	//TODO: ver si vamos hacer algo
	return 0;
}

int read_driver_keyboard(char *buf, unsigned size){
	bool notError = true;
	int i = 0;
	while (i < size && notError) {
		//notError = mt_kbd_getch(buf + i);
		mt_kbd_getch(buf + i, &notError);
		i++;
	}
	if(!notError)
		return ERR_DRIVER_ERROR;
	else 
		return notError;
}

int write_driver_keyboard(char *buf, unsigned size){
	bool b;
	mt_kbd_puts(buf, size,&b);
	if (b)
		return true;
	//if (mt_kbd_puts(buf, size))
	//	return true;
	return ERR_DRIVER_ERROR;
}

int close_driver_keyboard(void) {
	//TODO: ver si vamos a hacer algo
	return 0;
}

int ioctl_driver_keyboard(int type,int minor, ...) {
	
	int rta = 0;
	va_list pa;
	va_start(pa, minor);
	switch(type){
		case INPUT_PUT: {
			input_event_t* ev = va_arg(pa, input_event_t*);
			bool* b = va_arg(pa, bool*);
			mt_input_put(ev,b);
			break;
		}
		case INPUT_GET: {
			input_event_t* ev = va_arg(pa, input_event_t*);
			bool* b = va_arg(pa, bool*);
			mt_input_get(ev,b);
			break;
		}
		case INPUT_GET_COND: {
			input_event_t* ev = va_arg(pa, input_event_t*);
			bool* b = va_arg(pa, bool*);
			mt_input_get_cond(ev,b);
			break;
		}
		case INPUT_GET_TIMED: {
			input_event_t* ev = va_arg(pa, input_event_t*);
			unsigned timeout = va_arg(pa, unsigned);
			bool* b = va_arg(pa, bool*);
			mt_input_get_timed(ev,timeout,b);
			break;
		}
		case KBD_PUTS: {
			char* s = va_arg(pa, char*);
			unsigned len = va_arg(pa, unsigned);
			bool* b = va_arg(pa, bool*);
			mt_kbd_puts(s,len,b);
			break;
		}
		case KBD_PUTCH: {
			char* c = va_arg(pa, char*);
			bool* b = va_arg(pa, bool*);
			if(c!=NULL)
				mt_kbd_putch(*c,b);
			else
				rta = ERR_INVALID_ARGUMENT;
			break;
		}
		case KBD_GETCH: {
			char* c = va_arg(pa, char*);
			bool* b = va_arg(pa, bool*);
			mt_kbd_getch(c,b);
			break;
		}
		case KBD_GETCH_COND: {
			unsigned char* c = va_arg(pa, unsigned char*);
			bool* b = va_arg(pa, bool*);
			mt_kbd_getch_cond(c,b);
			break;
		}
		case KBD_GETCH_TIMED: {
			unsigned char* c = va_arg(pa, unsigned char*);
			unsigned timeout = va_arg(pa, unsigned);
			bool* b = va_arg(pa, bool*);
			mt_kbd_getch_timed(c,timeout,b);
			break;
		}
		case INPUT_SETFOCUS: {
			unsigned consnum = va_arg(pa, unsigned);
			mt_input_setfocus(consnum);
			break;
		}
		case INPUT_SETCURRENT: {
			unsigned consnum = va_arg(pa, unsigned);
			mt_input_setcurrent(consnum);
		}
		default:
			rta = ERR_NO_METHOD_EXIST;
	}
	va_end(pa);
	return rta;
}

int read_block_driver_keyboard(unsigned minor, unsigned block, unsigned nblocks, void *buffer) { 
	return ERR_NO_METHOD_EXIST;
}
	
int write_block_driver_keyboard(unsigned minor, unsigned block, unsigned nblocks, void *buffer) {
	return ERR_NO_METHOD_EXIST;
}

driver_t *generateDriver_keyboard() {
	driver_t *driver = malloc(sizeof(driver_t));
	driver->name = NAME_KEYBOARD;
	driver->read_driver = *read_driver_keyboard;
	driver->write_driver = *write_driver_keyboard;
	driver->ioctl_driver = *ioctl_driver_keyboard;
	driver->read_block_driver = *read_block_driver_keyboard;
	driver->write_block_driver = *write_block_driver_keyboard;
	return driver;
}