#include <stdio.h>
char	_sibuf[BUFSIZ];
char	_sobuf[BUFSIZ];

struct	_iobuf	_iob[_NFILE] = {
#ifdef	vax
	{ 0, _sibuf, _sibuf, _IOREAD, 0},
	{ 0, NULL, NULL, _IOWRT, 1},
	{ 0, NULL, NULL, _IOWRT+_IONBF, 2},
#endif
#ifdef z8000
	{ _sibuf, 0, _sibuf, _IOREAD, 0},
	{ NULL, 0, NULL, _IOWRT, 1},
	{ NULL, 0, NULL, _IOWRT+_IONBF, 2},
#endif
};
/*
 * Ptr to end of buffers
 */
struct	_iobuf	*_lastbuf = { &_iob[_NFILE] };
