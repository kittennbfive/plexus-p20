/*LINTLIBRARY*/
/*
 *	C library - ascii to floating
 */

#include <ctype.h>

double
atof(p)
register char *p;
{
	register c;
	double fl, flexp, exp5;
	double big = 9007199254740992.; /* 2^53 */
	double ldexp();
	register eexp, exp, neg, negexp, bexp;

	neg = 1;
	while(isspace(*p))
		++p;
	if(*p == '-') {
		++p;
		neg = -1;
	}
	else if(*p == '+')
		++p;

	exp = 0;
	fl = 0;
	while((c = *p++), isdigit(c))
		if(fl < big)
			fl = 10*fl + (c-'0');
		else
			exp++;

	if(c == '.')
		while((c = *p++), isdigit(c))
			if(fl < big) {
				fl = 10*fl + (c-'0');
				exp--;
			}

	negexp = 1;
	eexp = 0;
	if((c == 'E') || (c == 'e')) {
		if((c = *p++) == '+')
			;
		else if(c == '-')
			negexp = -1;
		     else
			--p;

		while((c = *p++), isdigit(c))
			eexp = 10*eexp + (c-'0');

		if(negexp < 0)
			eexp = -eexp;
		exp = exp + eexp;
	}

	negexp = 1;
	if(exp < 0) {
		negexp = -1;
		exp = -exp;
	}

	flexp = 1;
	exp5 = 5;
	bexp = exp;
	while(1) {
		if(exp&01)
			flexp *= exp5;
		exp >>= 1;
		if(exp == 0)
			break;
		exp5 *= exp5;
	}
	if(negexp < 0)
		fl /= flexp;
	else
		fl *= flexp;
	fl = ldexp(fl, negexp*bexp);
	if(neg < 0)
		fl = -fl;
	return(fl);
}
