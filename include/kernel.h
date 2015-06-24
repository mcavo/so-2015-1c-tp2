#ifndef KERNEL_H_INCLUDED
#define KERNEL_H_INCLUDED

#include <mtask.h>
#include <segments.h>
#include <const.h>
#include "drivers.h"


#define MTASK_VERSION	"20.03"		// Nro. de versión de MTask

// Registros empujados al stack por una interrupción o excepción.
// Esta estructura debe mantenerse sincronizada con el código de resguardo y
// recuperación de contexto en el manejador de interrupciones (interrupts.S)
// y con la función mt_context_switch() (libasm.S).
typedef struct
{
	unsigned 		edi;			// Registros empujados por pushal
	unsigned 		esi;			// .
	unsigned 		ebp;			// .
	unsigned 		esp;			// .
	unsigned 		ebx;			// .
	unsigned 		edx;			// .
	unsigned 		ecx;			// .
	unsigned 		eax;			// .
	unsigned 		eip;			// Registros empujados por el i386
	unsigned		cs;				// .
	unsigned		eflags;			// .
}
mt_regs_t;

// Colas de tareas
struct TaskQueue_t
{
	char *			name;
	Task_t *		head;
	Task_t *		tail;
};

// Bloque de control de una tarea
struct Task_t
{
	TaskQueue_t 	send_queue;
	TaskState_t		state;
	unsigned		priority;
	unsigned		atomic_level;
	mt_regs_t *		esp;			// Offset declarado en const.h
	char *			stack;
	TaskQueue_t	*	queue;
	Task_t *		prev;
	Task_t *		next;
	bool			success;
	bool			in_time_q;
	Task_t *		time_prev;
	Task_t *		time_next;
	unsigned		ticks;
	void *			tls;
	void *			math_data;
	SaveRestore_t	save;
	SaveRestore_t	restore;
	Task_t *		from;
	void *			msg;
	unsigned 		size;
	Cleanup_t		cleanup;
	unsigned		consnum;
	Task_t *		list_prev;
	Task_t *		list_next;
	Task_t *		join;
	int				join_status;
	Task_t *		attached_to;
	unsigned		nattached;
	bool			exiting;
	bool			protected;
};

/* malloc.c */

void mt_setup_heap(unsigned himem_size);

/* gdt_idt.c */

void mt_setup_gdt_idt(void);

/* interrupts.S */

typedef char int_stub[INT_STUB_SIZE];
extern int_stub mt_int_stubs[NUM_INTS];

/* libasm.S */

void mt_load_gdt(const region_desc *gdt);
void mt_load_idt(const region_desc *idt);
void mt_context_switch(void);
void mt_sti(void);
void mt_cli(void);
unsigned mt_flags(void);
void mt_finit(void);
void mt_fsave(void *buf);
void mt_frstor(void *buf);
void mt_stts(void);
void mt_clts(void);
void mt_hlt(void);

/* kernel.c */

typedef struct boot_info_t boot_info_t;

void mt_main(unsigned magic, boot_info_t *info);
bool mt_select_task(void);

extern Task_t * volatile mt_curr_task;
extern Task_t * volatile mt_last_task;
extern Task_t * volatile mt_fpu_task;

/* irq.c */

extern unsigned volatile mt_int_level;

void mt_int_handler(unsigned int_num, unsigned except_error, mt_regs_t *regs);

typedef void (*exception_handler)(unsigned except_number, unsigned error, mt_regs_t *regs);
typedef void (*interrupt_handler)(unsigned irq_number);

void mt_setup_interrupts(void);
void mt_set_int_handler(unsigned irq_num, interrupt_handler handler);
void mt_set_exception_handler(unsigned except_num, exception_handler handler);
void mt_enable_irq(unsigned irq);
void mt_disable_irq(unsigned irq);

/* timer.c */

void mt_setup_timer(unsigned freq);

/* queue.c */

void mt_enqueue(Task_t *task, TaskQueue_t *queue);
void mt_dequeue(Task_t *task);
Task_t *mt_peeklast(TaskQueue_t *queue);
Task_t *mt_getlast(TaskQueue_t *queue);

void mt_enqueue_time(Task_t *task, unsigned ticks);
void mt_dequeue_time(Task_t *task);
Task_t *mt_peekfirst_time(void);
Task_t *mt_getfirst_time(void);

/* math.c */

void mt_setup_math(void);

/* drivers.c */

void mt_init_drivers(void);

/* cons.c */

enum COLORS
{
	/* oscuros */
	BLACK,
	BLUE,
	GREEN,
	CYAN,
	RED,
	MAGENTA,
	BROWN,
	LIGHTGRAY,

	/* claros */
	DARKGRAY,
	LIGHTBLUE,
	LIGHTGREEN,
	LIGHTCYAN,
	LIGHTRED,
	LIGHTMAGENTA,
	YELLOW,
	WHITE
};

#define NVCONS 9

#define  CONS_INIT 				0
#define  CONS_CLEAR 			1
#define  CONS_CLREOL 			2
#define  CONS_CLREOM			3
#define  CONS_NROWS				4
#define  CONS_NCOLS				5
#define  CONS_NSCROLLS			6
#define  CONS_GETXY				7
#define  CONS_GOTOXY			8
#define  CONS_SETATTR			9
#define  CONS_GETATTR			10
#define  CONS_CURSOR			11
#define  CONS_RAW				12
#define  CONS_PUTC				13
#define  CONS_PUTS				14
#define  CONS_CR				15
#define  CONS_NL				16
#define  CONS_TAB				17
#define  CONS_BS				18
#define  CONS_SETFOCUS			19
#define  CONS_SETCURRENT		20
#define  CONS_SET0				21

int ioctl_driver_cons(int type,int minor, ...);

/* input.c */ 

enum { KBD_EVENT, MOUSE_EVENT };

typedef struct __attribute__((packed))
{
	unsigned char scan_codes[3];
}
kbd_event_t;

typedef struct __attribute__((packed))
{
	union __attribute__((packed))
	{
		struct __attribute__((packed))
		{
			unsigned char left_button	:1;
			unsigned char right_button	:1;
			unsigned char middle_button	:1;
			unsigned char always_1		:1;
			unsigned char x_sign		:1;
			unsigned char y_sign		:1;
			unsigned char x_ovfl		:1;
			unsigned char y_ovfl		:1;
		};
		unsigned char header;
	};
	unsigned char x;
	unsigned char y;
	signed   char z;
}
mouse_event_t;

typedef struct __attribute__((packed))
{
	unsigned char type;
	union
	{
		kbd_event_t kbd;
		mouse_event_t mouse;
	};
}
input_event_t;


#define  INPUT_INIT				0
#define  INPUT_PUT				1
#define  INPUT_GET 				2
#define  INPUT_GET_COND 		3
#define  INPUT_GET_TIMED 		4
#define  KBD_PUTCH				5
#define  KBD_PUTS				6
#define  KBD_GETCH 				7
#define  KBD_GETCH_COND			8
#define  KBD_GETCH_TIMED 		9
#define  INPUT_SETFOCUS			10
#define  INPUT_SETCURRENT		11

int ioctl_driver_keyboard(int type,int minor, ...);

//driver_t* mt_input_init(void);

bool mt_input_put(input_event_t *ev);
bool mt_input_get(input_event_t *ev);
bool mt_input_get_cond(input_event_t *ev);
bool mt_input_get_timed(input_event_t *ev, unsigned timeout);

bool mt_kbd_putch(char c);
bool mt_kbd_puts(char *s, unsigned len);
bool mt_kbd_getch(char *c);
bool mt_kbd_getch_cond(unsigned char *c);
bool mt_kbd_getch_timed(unsigned char *c, unsigned timeout);

void mt_input_setfocus(unsigned consnum);
void mt_input_setcurrent(unsigned consnum);

/* ps2.c */

#define  PS2_INIT 				0
#define  PS2_GETLAYOUT 			1 
#define  PS2_SETLAYOUT 			2
#define  PS2_LAYOUTS 			3

int ioctl_driver_ps2(int type,int minor, ...);

/* ide.c */

#define SECTOR_SIZE 512		// Tamaño de un sector

#define	 IDE_INIT 				0
#define	 IDE_READ 				1
#define	 IDE_WRITE 				2
#define	 IDE_MODEL 				3
#define	 IDE_CAPACITY 			4

int ioctl_driver_ide(int type,int minor, ...);

enum ide_minors				// Números de los discos
{
	IDE_PRI_MASTER,
	IDE_PRI_SLAVE,
	IDE_SEC_MASTER,
	IDE_SEC_SLAVE
};

#endif


