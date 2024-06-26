/*	Plexus - Sys3 - August 1982	*/

static char order_c[] = "@(#)order.c	1.1";

# include "mfile2"

int fltused = 0;
extern int odebug;
NODE * block();

stoasg( p, o ) register NODE *p; {
	/* should the assignment op p be stored,
	   given that it lies as the right operand of o
	   (or the left, if o==UNARY MUL) */
	return( shltype(p->in.left->in.op, p->in.left ) );

	}

deltest( p ) register NODE *p; {
	/* should we delay the INCR or DECR operation p */
	/* not yet ready to use push and pop */
	/*
	if( p->in.op == INCR && p->in.left->in.op == REG
	    && spsz( p->in.left->in.type, p->in.right->tn.lval ) ){
		*/ /* STARREG */ /*
		return( 0 );
		}

	*/
	p = p->in.left;
	/* if( p->in.op == UNARY MUL ) p = p->in.left; */
	return( p->in.op == NAME || p->in.op == OREG || p->in.op == REG );
	}

mkadrs(p) register NODE *p; {
	register o;
	register t;

	o = p->in.op;
	t = p->in.type;

	if( asgop(o) ){
		if( p->in.left->bn.su >= p->in.right->bn.su ){
			if( p->in.left->in.op == UNARY MUL ){
				if( p->in.left->bn.su > 0 )
					SETSTO( p->in.left->in.left, INTEMP );
				else {
					if( p->in.right->bn.su > 0 )
						SETSTO( p->in.right, INTEMP );
					else cerror( "store finds both sides trivial" );
					}
				}
			else if( p->in.left->in.op == FLD
			    && p->in.left->in.left->in.op == UNARY MUL ){
				SETSTO( p->in.left->in.left->in.left, INTEMP );
				}
			else { /* should be only structure assignment */
				SETSTO( p->in.left, INTEMP );
				}
			}
		else SETSTO( p->in.right, INTEMP );
		}
	else {
		if( p->in.left->bn.su > p->in.right->bn.su ){
			SETSTO( p->in.left, INTEMP );
			}
		else {
			SETSTO( p->in.right, INTEMP );
			}
		}
	}

notoff( t, r, off, cp) TWORD t; CONSZ off; char *cp; {
	/* is it legal to make an OREG or NAME entry which has an
	/* offset of off, (from a register of r), if the
	/* resulting thing had type t */

	if (r == R0) return(1);		/* NO */
	return(0);			/* YES */
	}

# define max(x,y) ((x)<(y)?(y):(x))
# define min(x,y) ((x)<(y)?(x):(y))


# define ZCHAR 01
# define ZLONG 02
# define ZFLOAT 04

zum( p, zap ) register NODE *p; {
	/* zap Sethi-Ullman number for chars, longs, floats */
	/* in the case of longs, only STARNM's are zapped */
	/* ZCHAR, ZLONG, ZFLOAT are used to select the zapping */

	register su;

	su = p->bn.su;

	switch( p->in.type ){

	case CHAR:
	case UCHAR:
		if( !(zap&ZCHAR) ) break;
		if( su == 0 ) p->bn.su = su = 1;
		break;

	case LONG:
	case ULONG:
		if( !(zap&ZLONG) ) break;
		if( p->in.op == UNARY MUL && su == 0 ) p->bn.su = su = 2;
		break;

	case FLOAT:
		break;
		}

	return( su );
	}

sucomp( p ) register NODE *p; {

	/* set the su field in the node to the sethi-ullman
	   number, or local equivalent */

	register o, ty, sul, sur;
	register nr;

/*
if (zdebug) fwalk(p,eprint,0);
*/
	ty = optype( o=p->in.op);
	nr = szty( p->in.type );
	p->bn.su = 0;

	if( ty == LTYPE ) {
		if( p->in.type == FLOAT || p->in.type == DOUBLE )
			p->bn.su = 4;
		return;
		}
	else if( ty == UTYPE ){
		switch( o ) {
		case UNARY CALL:
		case UNARY STCALL:
			p->bn.su = fregs;  /* all regs needed */
			return;

		case UNARY MUL:
			if( shumul( p->in.left ) ) return;

		default:
			p->bn.su = max( p->in.left->bn.su, nr);
			return;
			}
		}


	/* If rhs needs n, lhs needs m, regular su computation */

	sul = p->in.left->bn.su;
	sur = p->in.right->bn.su;

	if( o == ASSIGN ){
		asop:  /* also used for +=, etc., to memory */
		if( sul==0 ){
			/* don't need to worry about the left side */
			p->bn.su = max( sur, nr );
			}
		else {
			/* right, left address, op */
			if( sur == 0 ){
				/* just get the lhs address into a register,
				   and mov */
				/* the `nr' covers the case where value
				   is in reg afterwards */
				p->bn.su = max( sul, nr );
				}
			else {
				/* right, left address, op */
				p->bn.su = max( sur, nr+sul );
				}
			}
		return;
		}

	if( o == CALL || o == STCALL ){
		/* in effect, takes all free registers */
		p->bn.su = fregs;
		return;
		}

	if( o == STASG ){
		/* right, then left */
		p->bn.su = max( max( sul+nr, sur), fregs );
		return;
		}

	if( logop(o) ){
		/* do the harder side, then the easier side, into registers */
		/* left then right, max(sul,sur+nr) */
		/* right then left, max(sur,sul+nr) */
holdit();
		/* to hold both sides in regs: nr+nr */
		nr = szty( p->in.left->in.type );
		sul = zum( p->in.left, ZLONG|ZFLOAT );
		sur = zum( p->in.right, ZLONG|ZFLOAT );
		p->bn.su = min( max(sul,sur+nr), max(sur,sul+nr) );
		return;
		}

	if( asgop(o) ){
		/* computed by doing right, doing left address,
		   doing left, op, and store */
		switch( o ) {
		case INCR:
		case DECR:
			/* do as binary op */
			break;

		case ASG DIV:
		case ASG MOD:
		case ASG MUL:
/*
			if( p->in.type!=FLOAT && p->in.type!=DOUBLE ) nr *= 2;
*/
			goto gencase;

		case ASG PLUS:
		case ASG MINUS:
		case ASG AND:
		case ASG OR:
			if( p->in.type == INT || p->in.type == UNSIGNED
			    || ISPTR(p->in.type) ) goto asop;

		gencase:
		default:
			sur = zum( p->in.right, ZLONG|ZFLOAT );
			if( sur == 0 ){ /* easy case: if addressable,
				do left value, op, store */
				if( sul == 0 ) p->bn.su = nr;
				/* harder: left adr, val, op, store */
				else p->bn.su = max( sul, nr+1 );
				}
			else { /* do right, left adr, left value, op, store */
				if( sul == 0 ){  /* right, left value, op, store */
					p->bn.su = max( sur, nr+nr );
					}
				else {
					if (p->in.type == FLOAT
					    || p->in.type == DOUBLE)
					p->bn.su = max(sur, max( sul, 1+nr));
					else
					p->bn.su = max( sur, max( sul+nr, 1+nr+nr ) );
					}
				}
			return;
			}
		}

	switch( o ){
	case ANDAND:
	case OROR:
	case QUEST:
	case COLON:
	case COMOP:
		p->bn.su = max( max(sul,sur), nr);
		return;
		}

	if( ( o==DIV || o==MOD || o==MUL )
	    && p->in.type!=FLOAT && p->in.type!=DOUBLE ) nr *= 2;
	if( o==PLUS || o==MUL || o==AND || o==OR || o==ER ){
		/* permute: get the harder on the left */

		register rt, lt;

		if( istnode( p->in.left ) || sul > sur ) goto noswap;  /* don't do it! */

		/* look for a funny type on the left, one on the right */


		lt = p->in.left->in.type;
		rt = p->in.right->in.type;

		if( rt == FLOAT && lt == DOUBLE ) goto swap;

		if( (rt==CHAR||rt==UCHAR)
		    && (lt==INT||lt==UNSIGNED||ISPTR(lt)) ) goto swap;

		if( lt==LONG || lt==ULONG ){
			if( rt==LONG || rt==ULONG ){
				/* if one is a STARNM, swap */
				if( p->in.left->in.op == UNARY MUL && sul==0 )
					goto noswap;
				if( p->in.right->in.op == UNARY MUL
				    && p->in.left->in.op != UNARY MUL )
					goto swap;
				goto noswap;
				}
			else if( p->in.left->in.op == UNARY MUL && sul == 0 )
				goto noswap;
			else goto swap;  /* put long on right, unless STARNM */
			}

		/* we are finished with the type stuff now; if one is addressable,
			put it on the right */
		if( sul == 0 && sur != 0 ){

			NODE *s;
			int ssu;

		swap:
			ssu = sul;  sul = sur; sur = ssu;
			s = p->in.left;  p->in.left = p->in.right; p->in.right = s;
			}
		}
	noswap:

	sur = zum( p->in.right, ZLONG|ZFLOAT );
	if( (sur == 0) && p->in.type !=FLOAT && p->in.type != DOUBLE) {
		/* get left value into a register, do op */
		p->bn.su = max( nr, sul );
		}
	else {
		/* do harder into a register, then easier */
		p->bn.su = max( nr+nr, min( max( sul, nr+sur ), max( sur, nr+sul ) ) );
		}
	}

holdit() {;}
int radebug = 0;

mkrall( p, r ) register NODE *p; {
	/* insure that the use of p gets done with register r; in effect, */
	/* simulate offstar */

	if( p->in.op == FLD ){
		p->in.left->in.rall = p->in.rall;
		p = p->in.left;
		}

	if( p->in.op != UNARY MUL ) return;  /* no more to do */
	p = p->in.left;
	if( p->in.op == UNARY MUL ){
		p->in.rall = r;
		p = p->in.left;
		}
	if( p->in.op == PLUS && p->in.right->in.op == ICON ){
		p->in.rall = r;
		p = p->in.left;
		}
	rallo( p, r );
	}

rallo( p, down ) register NODE *p; {
	/* do register allocation */
	register o, type, down1, down2, ty;

	if( radebug ) printf( "rallo( %o, %o )\n", p, down );

	down2 = NOPREF;
	p->in.rall = down;
	down1 = ( down &= ~MUSTDO );

	ty = optype( o = p->in.op );
	type = p->in.type;


	if( type == DOUBLE || type == FLOAT ){
		if( o == FORCE ) down1 = R4|MUSTDO;
		++fltused;
		}
	else switch( o ) {
	case ASSIGN:	
		down1 = NOPREF;
		down2 = down;
		break;

	case ASG MUL:
	case ASG DIV:
	case ASG MOD:
	case MUL:
	case DIV:
	case MOD:
		return;

	case CALL:
	case STASG:
	case EQ:
	case NE:
	case GT:
	case GE:
	case LT:
	case LE:
	case NOT:
	case ANDAND:
	case OROR:
		down1 = NOPREF;
		break;

	case FORCE:	
		if (type == DOUBLE)
			down1 = R4|MUSTDO;
		else if ((type == LONG) || (type == ULONG))
			down1 = R6|MUSTDO;
		else
			down1 = R7|MUSTDO;
		break;

		}

	if( ty != LTYPE ) rallo( p->in.left, down1 );
	if( ty == BITYPE ) rallo( p->in.right, down2 );

	}

offstar( p ) register NODE *p; {
	/* handle indirections */

	if( p->in.op == UNARY MUL ) p = p->in.left;

	if( p->in.op == PLUS || p->in.op == MINUS ){
		if( p->in.right->in.op == ICON ){
			order( p->in.left , INTAREG|INAREG );
			return;
			}
		}
	order( p, INTAREG|INAREG );
	}

setincr( p ) NODE *p; {
	return( 0 );	/* for the moment, don't bother */
	}

niceuty( p ) register NODE *p; {
	register TWORD t;

	return( p->in.op == UNARY MUL && /* (t=p->in.type)!=CHAR && */
/*
		t!= UCHAR && t!= FLOAT &&
CCF for now */
/*		t != UCHAR && */
		shumul( p->in.left) != STARREG );
	}

setbin( p ) register NODE *p; {
	register NODE *r, *l;

	if(odebug) printf("setbin(%o)\n",p);
	r = p->in.right;
	l = p->in.left;
if (rdebug) {
	printf("setbin(%o), l->bn.su(%d), r->bn.su(%d)\n",p,l->bn.su,r->bn.su);
}

if(l->in.type != DOUBLE && l->in.type != FLOAT)
	if( p->in.right->bn.su == 0 ){ /* rhs is addressable */
		if( logop( p->in.op ) ){
			if( l->in.op == UNARY MUL && l->in.type != DOUBLE
			    && l->in.type!=FLOAT && shumul( l->in.left ) != STARREG )
				offstar( l->in.left );
			else order( l, INTAREG|INAREG|INTBREG|INBREG|INTEMP );
			return( 1 );
		}
		if( !istnode( l ) ){
			if( niceuty( l ) )
				offstar( l->in.left );
			else
				order( l, INTAREG|INTBREG );
			return( 1 );
		}
		/* rewrite */
		return( 0 );
	}
	/* now, rhs is complicated: must do both sides into registers */
	/* do the harder side first */

	if( logop( p->in.op ) ){
		/* relational: do both sides into regs if need be */

		if( r->bn.su > l->bn.su ){
			if( niceuty(r) ){
				offstar( r->in.left );
				return( 1 );
			}
			else if( !istnode( r ) ){
				order( r, INTAREG|INAREG|INTBREG|INBREG|INTEMP);
				return( 1 );
			}
		}
		if( niceuty(l) ){
			offstar( l->in.left );
			return( 1 );
		}
/*
		else if( niceuty(r) ){
			offstar( r->in.left );
			return( 1 );
		}
*/
		else if( !istnode( l ) ){
			order( l, INTAREG|INAREG|INTBREG|INBREG|INTEMP );
			return( 1 );
		}
		if( !istnode( r ) ){
			order( r, INTAREG|INAREG|INTBREG|INBREG|INTEMP );
			return( 1 );
		}
		cerror( "setbin can't deal with %s", opst[p->in.op] );
	}

	/* ordinary operator */

	if( !istnode(r) && r->bn.su > l->bn.su ){
		/* if there is a chance of making it addressable, try it... */
		if( niceuty(r) ){
			offstar( r->in.left );
			return( 1 );  /* hopefully, it is addressable by now */
		}
		order( r, INTAREG|INAREG|INTBREG|INBREG|INTEMP );  /* anything goes on rhs */
		return( 1 );
	}
	else {
		if( !istnode( l ) ){
			if(( l->in.type == FLOAT || l->in.type == DOUBLE )
			    && niceuty(l)) {
				offstar( l->in.left );
				return( 1 );
				/* hopefully, it is addressable by now */
			}
			order( l, INTAREG|INTBREG );
			return( 1 );
		}
		/* rewrite */
		return( 0 );
	}
}

setstr( p ) register NODE *p; { /* structure assignment */
	if(odebug) printf("setstr(%o)\n",p);
	if( p->in.right->in.op != REG ){
		order( p->in.right, INTAREG );
		return(1);
		}
	p = p->in.left;
	if( p->in.op != NAME && p->in.op != OREG ){
		if( p->in.op != UNARY MUL ) cerror( "bad setstr" );
		order( p->in.left, INTAREG );
		return( 1 );
		}
	return( 0 );
	}

setasg( p ) register NODE *p; {
	/* setup for assignment operator */

	if(odebug) printf("setasg(%o)\n",p);
	if( p->in.right->bn.su != 0 && p->in.right->in.op != REG ) {
		if( p->in.right->in.op == UNARY MUL )
			offstar( p->in.right->in.left );
		else
			order( p->in.right, INTAREG|INAREG|INTBREG|INBREG|SOREG|SNAME|SCON );
		return(1);
		}
	if( p->in.right->in.op != REG && ( p->in.type == FLOAT || p->in.type == DOUBLE ) ) {
		order( p->in.right, INTAREG );
		return(1);
		}
	if( p->in.left->in.op == UNARY MUL && !tshape( p->in.left, STARREG|STARNM ) ){
		offstar( p->in.left->in.left );
		return(1);
		}
	if( p->in.left->in.op == FLD && p->in.left->in.left->in.op == UNARY MUL ){
		offstar( p->in.left->in.left->in.left );
		return(1);
		}
	/* if things are really strange, get rhs into a register */
	if( p->in.right->in.op != REG ){
		order( p->in.right, INTAREG|INAREG|INTBREG|INBREG );
		return( 1 );
		}
	return(0);
	}

setasop( p ) register NODE *p; {
	/* setup for =ops */
	register sul, sur;
	register NODE *q, *p2;

	if(odebug) printf("setasop(%o)\n",p);
	sul = p->in.left->bn.su;
	sur = p->in.right->bn.su;

	/*
	switch( p->in.op ){

	case ASG PLUS:
	case ASG OR:
	case ASG MINUS:
		if( p->in.type != INT && p->in.type != UNSIGNED
		    && !ISPTR(p->in.type) ) break;
		if( p->in.right->in.type == CHAR || p->in.right->in.type == UCHAR ){
			order( p->in.right, INTAREG|INAREG );
			return( 1 );
		}
	}
	*/

	if( sur == 0 ){

	if( ( p->in.op == ASG LS || p->in.op == ASG RS )
	    && p->in.right->in.op != ICON && !istnode( p->in.right ) ) {
		order( p->in.right, INTAREG );
		return( 1 );
	}
	leftadr:
		/* easy case: if addressable, do left value, op, store */
		if( sul == 0 ) goto rew;  /* rewrite */

		/* harder; make aleft address, val, op, and store */

		/*ERR*/
		if( (p->in.type == FLOAT || p->in.type == DOUBLE)
		     && p->in.left->in.op == UNARY MUL )
			return( 0 );
		if( p->in.left->in.op == UNARY MUL ){
			offstar( p->in.left->in.left );
			return( 1 );
		}
		if( p->in.left->in.op == FLD && p->in.left->in.left->in.op == UNARY MUL ){
			offstar( p->in.left->in.left->in.left );
			return( 1 );
		}
	rew:	/* for Z8000, autoincrement and autodecrement are not */
		/*   allowed, so we will return here */

		return(0);
	}

	/* harder case: do right, left address, left value, op, store */

	if( p->in.right->in.op == UNARY MUL ){
		offstar( p->in.right->in.left );
		return( 1 );
	}
	/* sur> 0, since otherwise, done above */
	if( p->in.right->in.op == REG ) goto leftadr;  /* make lhs addressable */
	order( p->in.right, INTAREG|INAREG|INTBREG|INBREG );
	return( 1 );
}

int crslab = 10000;
int tcc_ok;
getlab(){
	tcc_ok = 0;
	return( crslab++ );
	}

deflab( l ){
	printf( "L%d:\n", l );
	}

genargs( p) register NODE *p; {
	register size;
	register NODE *l;

	/* generate code for the arguments */
	if(odebug) printf( "genargs(%o)\n",p );

	/*  first, do the arguments on the right (last->first) */
	while( p->in.op == CM ){
		genargs( p->in.right );
		p->in.op = FREE;
		p = p->in.left;
	}

	if( p->in.op == STARG ){ /* structure valued argument */
#define PUSHSZ 12

		size = p->stn.stsize;
		l = p->in.left;
		if( l->in.op == ICON ){
			/* make into a name node */
			l->in.op = NAME;
			if( size <= PUSHSZ ) {
				p->in.op = FREE;
				p = l;
			}
		}
#ifdef ONEPASS
		else if( size > PUSHSZ ) {
			/* make it look beautiful... */
			/* insert a UNARY MUL node */
			p->in.left = l = block( UNARY MUL, l, p->in.right,
					p->in.type, 0, 0 );
			canon( l );  /* turn it into an oreg */
			if( l->in.op != OREG ){
				offstar( l->in.left );
				canon( l );
				if( l->in.op != OREG ) cerror( "stuck starg" );
			}
		}
#endif
		else {
			/* make it look beautiful... */
			p->in.op = UNARY MUL;
			canon( p );  /* turn it into an oreg */
			if( p->in.op != OREG ){
				offstar( p->in.left );
				canon( p );
				if( p->in.op != OREG ) cerror( "stuck starg" );
			}
		}

#ifdef ONEPASS
		if( size <= PUSHSZ ) {
#endif
			p->tn.lval += size;  /* end of structure */
			/* put on stack backwards */
			if( size & 2 ) { /* odd number of words */
				p->tn.lval -= 2;
				expand( p, RNOP, "#$\tpush\t*r15,ARZ-#)\n" );
				size -= 2;
			}
			for( ; size>0; size -= 4 ){
				p->tn.lval -= 4;
				expand( p, RNOP, "#$\tpushl\t*r15,ARZ-Z-#)\n" );
			}
			reclaim( p, RNULL, 0 );
			return;
		}
#ifdef ONEPASS
	}
#endif

	/* ordinary case */

	order( p, FORARG );
}

argsize( p ) register NODE *p; {
	register t;
	t = 0;
	if( p->in.op == CM ){
		t = argsize( p->in.left );
		p = p->in.right;
		}
	if( p->in.type == DOUBLE || p->in.type == FLOAT ){
		SETOFF( t, 2 );
		return( t+8 );
		}
	else if( p->in.type == LONG || p->in.type == ULONG ) {
		SETOFF( t, 2);
		return( t+4 );
		}
	else if( p->in.op == STARG ){
		SETOFF( t, p->stn.stalign );  /* alignment */
		return( t + p->stn.stsize );  /* size */
		}
	else {
		SETOFF( t, 2 );
		return( t+2 );
		}
	}
