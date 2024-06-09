#
/*
 *
 *	UNIX debugger - common definitions
 *
 */



/*	Layout of a.out file (fsym):
 *
 *	header of 8 words	magic number 405, 407, 410, 411, 430, 431
 *				text size	)
 *				data size	) in bytes but even
 *				bss size	)
 *				symbol table size
 *				entry point
 *				{unused}
 *				flag set if no relocation
 *	overlay header (if type 430, 431),
 *		8 words long:	maximum overlay segement size	)
 *				overlay 1 size			)
 *				overlay 2 size			)
 *				overlay 3 size			)
 *				overlay 4 size			) size
 *				overlay 5 size			) in
 *				overlay 6 size			) even
 *				overlay 7 size			) bytes
 *
 *	header:		0
 *	text:		16 (32 if type 430,431)
 *	overlays (if 430, 431 only):
 *			32+textsize
 *	overlaysize:	ov1+ov2+...+ov7
 *	data:		16+textsize (+overlaysize+16 if type 431, 431)
 *	relocation:	16+textsize+datasize (should not be present in 430,431)
 *	symbol table:	16+2*(textsize+datasize) or 16+textsize+datasize
 *	(if 430, 431):	32+(textsize+overlaysize+datasize)
 *
 */


#include <sys/param.h>
#include <sys/dir.h>
#ifndef OVKRNL
#include <sys/reg.h>
#endif
#include <sys/user.h>
#include <termio.h>
#include "mac.h"
#include "mode.h"


/*
 * Internal variables ---
 *  They are addressed by name. (e.g. (`0'-`9', `a'-`b'))
 *  thus there can only be 36 of them.
 */

#define VARB	11
#ifdef OVKRNL
#define VARC	12	/* current overlay number */
#endif
#define VARD	13
#define VARE	14
#define VARM	22
#ifdef OVKRNL
#define	VARO	24	/* overlay text segement addition */
#endif
#define VARS	28
#define VART	29

#ifndef OVKRNL
#define COREMAGIC 0140000
#define USERPS	2*(512-1)
#define USERPC	2*(512-2)
#endif

#define RD	0
#define WT	1
#define NSP	0
#define	ISP	1
#define	DSP	2
#define STAR	4
#define STARCOM 0200
#define DSYM	7
#define ISYM	2
#define ASYM	1
#define NSYM	0
#define ESYM	(-1)
#define BKPTSET	1
#define BKPTEXEC 2
#define	SYMSIZ	100
#define MAXSIG	20

#define BPT	0x7fff
#define FD	0200
#define	SETTRC	0
#define	RDUSER	2
#define	RIUSER	1
#define	WDUSER	5
#define WIUSER	4
#define	RUREGS	3
#define	WUREGS	6
#define	CONTIN	7
#define	SINGLE	9
#define	EXIT	8
#define NUMREGS 18
#define NUMFREGS 2

/*
 *	Register offsets from bottom of kernel stack
 */

#define pc	-1
#define fcw	-3
#define id	-4
#define r7	-5
#define r6	-6
#define r5	-7
#define r4	-8
#define r3	-9
#define r2	-10
#define r1	-11
#define r0	-12
#define r15	-13
#define sp	-13
#define dev	-14
#define r14	-16
#define r13	-17
#define r12	-18
#define r11	-19
#define r10	-20
#define r9	-21
#define r8	-22



#define NREG	14	/* 8 regs + PS from kernel stack */
#define MAXOFF	255
#define MAXPOS	80
#define MAXLIN	128
#define EOF	0
#define EOR	'\n'
#define TB	'\t'
#define QUOTE	0200
#define STRIP	0177
#define LOBYTE	0377
#define EVEN	-2


/* long to ints and back (puns) */
union {
	INT	I[2];
	L_INT	L;
} itolws;

#define leng(a)		((long)((unsigned)(a)))
#define shorten(a)	((int)(a))
#define itol(a,b)	(itolws.I[0]=(a), itolws.I[1]=(b), itolws.L)



/* result type declarations */
L_INT		inkdot();
SYMPTR		lookupsym();
SYMPTR		symget();
POS		get();
POS		chkget();
STRING		exform();
L_INT		round();
BKPTR		scanbkpt();
VOID		fault();

typedef struct termio TTY;
TTY	adbtty, usrtty;
#include <setjmp.h>
jmp_buf erradb;
