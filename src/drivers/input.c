#include <kernel.h>

#define INPUTSIZE 32
#define NAME_KEYBOARD "Keyboard"

static MsgQueue_t *events[NVCONS];
static MsgQueue_t *keys[NVCONS];
static MsgQueue_t *input_focus, *input_current, *key_focus, *key_current;
static unsigned focus, current;
static driver_t *generateDriver_keyboard();

driver_t* 
mt_input_init(void)
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

bool 
mt_input_put(input_event_t *ev)
{
	return PutMsgQueueCond(input_focus, ev);
}

bool 
mt_input_get(input_event_t *ev)
{
	return GetMsgQueue(input_current, ev);
}

bool 
mt_input_get_cond(input_event_t *ev)
{
	return GetMsgQueueCond(input_current, ev);
}

bool 
mt_input_get_timed(input_event_t *ev, unsigned timeout)
{
	return GetMsgQueueTimed(input_current, ev, timeout);
}

bool
mt_kbd_putch(char c)
{
	return PutMsgQueueCond(key_focus, &c);
}

bool
mt_kbd_puts(char *s, unsigned len)	
{
	Atomic();
	if ( INPUTSIZE - AvailMsgQueue(key_focus) < len )
	{
		Unatomic();
		return false;
	}
	while ( len-- )
		PutMsgQueueCond(key_focus, s++);
	Unatomic();
	return true;
}

bool 
mt_kbd_getch(char *c)
{
	return GetMsgQueue(key_current, c);
}

bool 
mt_kbd_getch_cond(unsigned char *c)
{
	return GetMsgQueueCond(key_current, c);
}

bool 
mt_kbd_getch_timed(unsigned char *c, unsigned timeout)
{
	return GetMsgQueueTimed(key_current, c, timeout);
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
		mt_cons_setfocus(focus);
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
	mt_cons_setcurrent(current);
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
		notError = mt_kbd_getch(buf + i);
		i++;
	}
	if(!notError)
		return DRIVER_ERROR;
	else 
		return notError;
}

int write_driver_keyboard(char *buf, unsigned size){
	if (mt_kbd_puts(buf, size))
		return true;
	return DRIVER_ERROR;
}

int close_driver_keyboard(void) {
	//TODO: ver si vamos a hacer algo
	return 0;
}

int ioctl_driver_keyboard(void) {
	//TODO: copiar prototipo y funcionamiento de printf
	return 0;
}

int read_block_driver_keyboard(unsigned minor, unsigned block, unsigned nblocks, void *buffer) { 
	return NO_METHOD_EXIST;
}
	
int write_block_driver_keyboard(unsigned minor, unsigned block, unsigned nblocks, void *buffer) {
	return NO_METHOD_EXIST;
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