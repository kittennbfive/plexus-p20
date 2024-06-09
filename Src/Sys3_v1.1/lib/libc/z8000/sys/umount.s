#include <syscall.h>

/*
 * umount
 */

.globl	_umount
.globl	cerror

_umount:
push	*r15, r14
ld	r14, r15
ld	r7, 4(r14)
ld	t0, r7
sc	$INDIR
.word	t9
jp	c, cerror
clr	r7
pop	r14, *r15
ret

.data

t9:
sc	$UMOUNT

t0:
.word	0
