#include <kernel.h>

#define VIDMEM 0xB8000
#define NUMROWS 25
#define NUMCOLS 80
#define VIDSIZE (2 * NUMROWS * NUMCOLS)
#define TABSIZE 8

#define CRT_ADDR 0x3D4
#define CRT_DATA 0x3D5
#define CRT_CURSOR_START 0x0A
#define CRT_CURSOR_END 0x0B
#define CRT_CURSOR_HIGH 0x0E
#define CRT_CURSOR_LOW 0x0F

#define DEFATTR ((BLACK << 12) | (LIGHTGRAY << 8))

#define BS 0x08
#define NAME_CONS "Cons"

typedef unsigned short row[NUMCOLS];

typedef struct
{
	row *vidmem;
	struct
	{
		unsigned cur_x;
		unsigned cur_y;
		unsigned cur_attr;
		bool cursor_on;
		bool raw;
		unsigned scrolls;
	}
	status;
}
console;

static console real_console = 
{
	.vidmem = (row *) VIDMEM,
	{
		.cur_attr = DEFATTR, 
		.cursor_on = true
	}
};

static console vcons[NVCONS];

static unsigned focus;
static unsigned current;
static console *cons = &real_console;
static driver_t *generateDriver_cons();

static void 
init_vcons(unsigned n)
{
	console *cons = &vcons[n];
	cons->vidmem = Malloc(VIDSIZE);
	memcpy(cons->vidmem, real_console.vidmem, VIDSIZE);
	memcpy(&cons->status, &real_console.status, sizeof cons->status);
}

static void 
show_real(void)
{
	outb(CRT_ADDR, CRT_CURSOR_END);
	outb(CRT_DATA, real_console.status.cursor_on ? 15 : 0);
}

static void
set_real(void)
{
	if (!real_console.status.cursor_on)
		return;
	unsigned off = real_console.status.cur_y * NUMCOLS + real_console.status.cur_x;
	outb(CRT_ADDR, CRT_CURSOR_HIGH);
	outb(CRT_DATA, off >> 8);
	outb(CRT_ADDR, CRT_CURSOR_LOW);
	outb(CRT_DATA, off);
}

static void
save(void)
{
	memcpy(vcons[focus].vidmem, real_console.vidmem, VIDSIZE);
	vcons[focus].status = real_console.status;	
}

static void
restore(void)
{
	memcpy(real_console.vidmem, vcons[focus].vidmem, VIDSIZE);
	real_console.status = vcons[focus].status;	
	show_real();
	set_real();
}

static inline void
set_cons(void)
{
	cons = current == focus ? &real_console : &vcons[current];
}

static inline void
show_cursor(void)
{
	if (cons == &real_console)
		show_real();
}

static inline void
set_cursor(void)
{
	if (cons == &real_console)
		set_real();
}

#define	vidmem		cons->vidmem
#define	cur_x		cons->status.cur_x
#define	cur_y		cons->status.cur_y
#define	cur_attr	cons->status.cur_attr
#define	cursor_on	cons->status.cursor_on
#define	raw			cons->status.raw
#define	scrolls		cons->status.scrolls

static void
scroll(void)
{
	int j;
	for (j = 1; j < NUMROWS; j++)
		memcpy(&vidmem[j - 1], &vidmem[j], sizeof(row));
	for (j = 0; j < NUMCOLS; j++)
		vidmem[NUMROWS - 1][j] = DEFATTR;
	scrolls++;
}

static void
put(unsigned char ch)
{
	if (cur_x >= NUMCOLS)
	{
		cur_x = 0;
		if (cur_y == NUMROWS - 1)
			scroll();
		else
			cur_y++;
	}
	vidmem[cur_y][cur_x++] = (ch & 0xFF) | cur_attr;
	set_cursor();
}

/* Interfaz pública */

void
mt_cons_clreol(void)
{
	bool ints = SetInts(false);
	unsigned short *p1 = &vidmem[cur_y][cur_x];
	unsigned short *p2 = &vidmem[cur_y + 1][0];
	while (p1 < p2)
		*p1++ = DEFATTR;
	SetInts(ints);
}

void
mt_cons_clreom(void)
{
	bool ints = SetInts(false);
	unsigned short *p1 = &vidmem[cur_y][cur_x];
	unsigned short *p2 = &vidmem[NUMROWS][0];
	while (p1 < p2)
		*p1++ = DEFATTR;
	SetInts(ints);
}

int
mt_cons_nrows(unsigned * numrows)
{
	if (numrows == NULL)
		return ERR_INVALID_ARGUMENT;
	* numrows = NUMROWS;
	return 0;
}

int
mt_cons_ncols(unsigned * numcols)
{
	if (numcols == NULL)
		return ERR_INVALID_ARGUMENT;
	* numcols = NUMCOLS;
	return 0 ;
}

int
mt_cons_nscrolls(unsigned * numscrolls)
{
	if (numscrolls == NULL)
		return ERR_INVALID_ARGUMENT;
	* numscrolls = scrolls;
	return 0;
}

int
mt_cons_getxy(unsigned *x, unsigned *y)
{
	if (x == NULL || y == NULL)
		return ERR_INVALID_ARGUMENT;
	bool ints = SetInts(false);
	*x = cur_x;
	*y = cur_y;
	SetInts(ints);
	return 0;
}

void
mt_cons_gotoxy(unsigned x, unsigned y)
{
	bool ints = SetInts(false);
	if (y < NUMROWS && x < NUMCOLS)
	{
		cur_x = x;
		cur_y = y;
		set_cursor();
	}
	SetInts(ints);
}

void
mt_cons_setattr(unsigned fg, unsigned bg)
{
	cur_attr = ((fg & 0xF) << 8) | ((bg & 0xF) << 12);
}

int
mt_cons_getattr(unsigned *fg, unsigned *bg)
{
	if (fg == NULL || bg == NULL)
		return ERR_INVALID_ARGUMENT;
	bool ints = SetInts(false);
	*fg = (cur_attr >> 8) & 0xF;
	*bg = (cur_attr >> 12) & 0xF;
	SetInts(ints);
	return 0;
}

void
mt_cons_cursor(bool on, bool * prevcursor)
{	
	bool ints = SetInts(false);
	bool prev = cursor_on;
	cursor_on = on;
	show_cursor();
	set_cursor();
	SetInts(ints);
	if (prevcursor != NULL)
		* prevcursor = prev;
}

void
mt_cons_raw(bool on, bool * prevraw)
{
	bool ints = SetInts(false);
	bool prev = raw;
	raw = on;
	SetInts(ints);
	if (prevraw!=NULL)
		* prevraw = prev;
}

void
mt_cons_cr(void)
{
	bool ints = SetInts(false);
	cur_x = 0;
	set_cursor();
	SetInts(ints);
}

void
mt_cons_nl(void)
{
	bool ints = SetInts(false);
	if (cur_y == NUMROWS - 1)
		scroll();
	else
		cur_y++;
	set_cursor();
	SetInts(ints);
}

void
mt_cons_tab(void)
{
	bool ints = SetInts(false);
	unsigned nspace = TABSIZE - (cur_x % TABSIZE);
	while (nspace--)
		put(' ');
	SetInts(ints);
}

void
mt_cons_bs(void)
{
	bool ints = SetInts(false);
	if (cur_x)
		cur_x--;
	else if (cur_y)
	{
		cur_y--;
		cur_x = NUMCOLS - 1;
	}
	set_cursor();
	SetInts(ints);
}

void
mt_cons_putc(char ch)
{
	bool ints = SetInts(false);
	if (raw)
	{
		put(ch);
		SetInts(ints);
		return;
	}
	switch (ch)
	{
		case '\t':
			mt_cons_tab();
			break;

		case '\r':
			mt_cons_cr();
			break;

		case '\n':
			mt_cons_nl();
			break;

		case BS:
			mt_cons_bs();
			break;

		default:
			put(ch);
			break;
	}
	SetInts(ints);
}

void
mt_cons_puts(const char *str)
{
	bool ints = SetInts(false);
	while (*str)
		mt_cons_putc(*str++);
	SetInts(ints);
}

// Se llama en modo atómico y no desde interrupciones, salvo Panic()
void
mt_cons_setfocus(unsigned consnum)
{
	save();
	focus = consnum;
	restore();
	set_cons();
}

// Se llama con interrupciones deshabilitadas
void
mt_cons_setcurrent(unsigned consnum)
{
	current = consnum;
	set_cons();
}

// Se llama con interrupciones deshabilitadas
void
mt_cons_set0(unsigned * prevv)
{
	unsigned prev = current;
	current = 0;
	set_cons();
	if (prevv != NULL)
		* prevv = prev;
}

driver_t *
mt_cons_init()
{
	unsigned i;

	// Inicializar línea de origen del cursor
	outb(CRT_ADDR, CRT_CURSOR_START);
	outb(CRT_DATA, 14);

	// Mostrar cursor
	mt_cons_cursor(true,NULL);

	// Inicializar las consolas virtuales
	for (i = 0; i < NVCONS; i++)
		init_vcons(i);
	return generateDriver_cons();
}

void
mt_cons_clear(void)
{
	bool ints = SetInts(false);
	unsigned short *p1 = &vidmem[0][0];
	unsigned short *p2 = &vidmem[NUMROWS][0];
	while (p1 < p2)
		*p1++ = DEFATTR;
	mt_cons_gotoxy(0, 0);
	SetInts(ints);
}

/* driver interface */

int open_driver_cons(void){
	return 0;
}

int read_driver_cons(char *buf, unsigned size){
	return ERR_NO_METHOD_EXIST;
}

/* size only must be 0 if you want to put all the string. If size = 1 putc is used */
int write_driver_cons(char *buf, unsigned size){
	if (size == 0)
		mt_cons_puts(buf);
	else {
		char c = *buf;
		mt_cons_putc(c);
	}
	return 0;
}

int close_driver_cons(void) {
	return 0;
}

int ioctl_driver_cons(int type,int minor, ...) {
	int rta = 0;
	va_list pa;
	va_start(pa, minor);
	switch(type){
		case CONS_CLEAR: {
			if (minor!=0)
				rta = ERR_INVALID_ARGUMENT;
			else
				mt_cons_clear();
			break;
		}
		case CONS_CLREOL: {
			if (minor!=0)
				rta = ERR_INVALID_ARGUMENT;
			else
				mt_cons_clreol();
			break;
		}
		case CONS_CLREOM: {
			if (minor!=0)
				rta = ERR_INVALID_ARGUMENT;
			else
				mt_cons_clreom();
			break;
		}
		case CONS_NROWS: {
			if (minor!=1)
				rta = ERR_INVALID_ARGUMENT;
			else {
				unsigned* numrows = va_arg(pa, unsigned*);
				rta = mt_cons_nrows(numrows);
			}
			break;
		}
		case CONS_NCOLS: {
			if (minor!=1)
				rta = ERR_INVALID_ARGUMENT;
			else {
				unsigned* numcols = va_arg(pa, unsigned*);
				rta = mt_cons_ncols(numcols);
			}
			break;
		}
		case CONS_NSCROLLS: {
			if (minor!=1)
				rta = ERR_INVALID_ARGUMENT;
			else {
				unsigned* numscrolls = va_arg(pa, unsigned*);
				rta = mt_cons_nscrolls(numscrolls);
			}
			break;
		}
		case CONS_GETXY: {
			if (minor!=2)
				rta = ERR_INVALID_ARGUMENT;
			else {
				unsigned* x = va_arg(pa, unsigned*);
				unsigned* y = va_arg(pa, unsigned*);
				rta = mt_cons_getxy(x, y);
			}
			break;
		}
		case CONS_GOTOXY: {
			if (minor!=2)
				rta = ERR_INVALID_ARGUMENT;
			else {	
				unsigned x = va_arg(pa, unsigned);
				unsigned y = va_arg(pa, unsigned);
				mt_cons_gotoxy(x, y);
			}
			break;
		}
		case CONS_SETATTR: {
			if (minor!=2)
				rta = ERR_INVALID_ARGUMENT;
			else {
				unsigned fg = va_arg(pa, unsigned);
				unsigned bg = va_arg(pa, unsigned);
				mt_cons_setattr(fg, bg);
			}
			break;
		}
		case CONS_GETATTR: {
			if (minor!=2)
				rta = ERR_INVALID_ARGUMENT;
			else {
				unsigned* fg = va_arg(pa, unsigned*);
				unsigned* bg = va_arg(pa, unsigned*);
				rta = mt_cons_getattr(fg, bg);
			}
			break;
		}
		case CONS_CURSOR: {
			if (minor!=2)
				rta = ERR_INVALID_ARGUMENT;
			else {
				bool on = va_arg(pa, bool);
				bool* curet = va_arg(pa, bool*);
				mt_cons_cursor(on,curet);
			}
			break;
		}
		case CONS_CR: {
			if (minor!=0)
				rta = ERR_INVALID_ARGUMENT;
			else
				mt_cons_cr();
			break;
		}
		case CONS_NL: {
			if (minor!=0)
				rta = ERR_INVALID_ARGUMENT;
			else
				mt_cons_nl();
			break;
		}
		case CONS_TAB: {
			if (minor!=0)
				rta = ERR_INVALID_ARGUMENT;
			else
				mt_cons_tab();
			break;
		}
		case CONS_BS: {
			if (minor!=0)
				rta = ERR_INVALID_ARGUMENT;
			else
				mt_cons_bs();
			break;
		}
		case CONS_RAW: {
			bool on = va_arg(pa, bool);
			bool* rawret = va_arg(pa, bool*);
			mt_cons_raw(on, rawret);
			break;
		}
		case CONS_SETFOCUS: {
			if (minor!=1)
				rta = ERR_INVALID_ARGUMENT;
			else {
				unsigned consnum = va_arg(pa, unsigned);
				mt_cons_setfocus(consnum);
			}
			break;
		}
		case CONS_SETCURRENT: {
			if (minor!=1)
				rta = ERR_INVALID_ARGUMENT;
			else {
				unsigned consnum = va_arg(pa, unsigned);
				mt_cons_setcurrent(consnum);
			}
			break;
		}
		case CONS_SET0: {
			if (minor!=1)
				rta = ERR_INVALID_ARGUMENT;
			else {
				unsigned* consnum = va_arg(pa, unsigned*);
				mt_cons_set0(consnum);
			}
			break;
		}
		default:
			rta = ERR_NO_METHOD_EXIST;
	}
	va_end(pa);
	return rta;

}

int read_block_driver_cons(unsigned minor, unsigned block, unsigned nblocks, void *buffer) { 
	return ERR_NO_METHOD_EXIST;
}
	
int write_block_driver_cons(unsigned minor, unsigned block, unsigned nblocks, void *buffer) {
	return ERR_NO_METHOD_EXIST;
}

driver_t *generateDriver_cons() {
	driver_t *driver = malloc(sizeof(driver_t));
	driver->name = NAME_CONS;
	driver->read_driver = *read_driver_cons;
	driver->write_driver = *write_driver_cons;
	driver->ioctl_driver = *ioctl_driver_cons;
	driver->read_block_driver = *read_block_driver_cons;
	driver->write_block_driver = *write_block_driver_cons;
	return driver;
}
