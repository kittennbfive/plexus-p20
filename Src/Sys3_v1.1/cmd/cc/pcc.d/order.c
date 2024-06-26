# include "mfile2"

int fltused = 0;
extern int odebug;

stoasg( p, o ) register NODE *p; {
	/* should the assignment op p be stored,
	   given that it lies as the right operand of o
	   (or the left, if o==UNARY MUL) */
	return( shltype(p->left->op, p->left ) );

	}

deltest( p ) register NODE *p; {
	/* should we delay the INCR or DECR operation p */
	/* not yet ready to use push and pop */
	/*
	if( p->op == INCR && p->left->op == REG
	    && spsz( p->left->type, p->right->lval ) ){
		*/ /* STARREG */ /*
		return( 0 );
		}

	*/
	p = p->left;
	/* if( p->op == UNARY MUL ) p = p->left; */
	return( p->op == NAME || p->op == OREG || p->op == REG );
	}

mkadrs(p) register NODE *p; {
	register o;
	register t;

	o = p->op;
	t = p->type;

/*CCF
	if (( t==FLOAT ) || ( t==DOUBLE)) return;
*/
	if( asgop(o) ){
		if( p->left->su >= p->right->su ){
			if( p->left->op == UNARY MUL ){
				if( p->left->su > 0 )
					SETSTO( p->left->left, INTEMP );
				else {
					if( p->right->su > 0 )
						SETSTO( p->right, INTEMP );
					else cerror( "store finds both sides trivial" );
					}
				}
			else if( p->left->op == FLD
			    && p->left->left->op == UNARY MUL ){
				SETSTO( p->left->left->left, INTEMP );
				}
			else { /* should be only structure assignment */
				SETSTO( p->left, INTEMP );
				}
			}
		else SETSTO( p->right, INTEMP );
		}
	else {
		if( p->left->su > p->right->su ){
			SETSTO( p->left, INTEMP );
			}
		else {
			SETSTO( p->right, INTEMP );
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

	su = p->su;

	switch( p->type ){

	case CHAR:
	case UCHAR:
		if( !(zap&ZCHAR) ) break;
		if( su == 0 ) p->su = su = 1;
		break;

	case LONG:
	case ULONG:
		if( !(zap&ZLONG) ) break;
		if( p->op == UNARY MUL && su == 0 ) p->su = su = 2;
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
	ty = optype( o=p->op);
	nr = szty( p->type );
	p->su = 0;

	if( ty == LTYPE ) {
		if( p->type == FLOAT || p->type == DOUBLE )
			p->su = 4;
		return;
		}
	else if( ty == UTYPE ){
		switch( o ) {
		case UNARY CALL:
		case UNARY STCALL:
			p->su = fregs;  /* all regs needed */
			return;

		case UNARY MUL:
			if( shumul( p->left ) ) return;

		default:
			p->su = max( p->left->su, nr);
			return;
			}
		}


	/* If rhs needs n, lhs needs m, regular su computation */

	sul = p->left->su;
	sur = p->right->su;

	if( o == ASSIGN ){
		asop:  /* also used for +=, etc., to memory */
		if( sul==0 ){
			/* don't need to worry about the left side */
			p->su = max( sur, nr );
			}
		else {
			/* right, left address, op */
			if( sur == 0 ){
				/* just get the lhs address into a register,
				   and mov */
				/* the `nr' covers the case where value
				   is in reg afterwards */
				p->su = max( sul, nr );
				}
			else {
				/* right, left address, op */
				p->su = max( sur, nr+sul );
				}
			}
		return;
		}

	if( o == CALL || o == STCALL ){
		/* in effect, takes all free registers */
		p->su = fregs;
		return;
		}

	if( o == STASG ){
		/* right, then left */
		p->su = max( max( sul+nr, sur), fregs );
		return;
		}

	if( logop(o) ){
		/* do the harder side, then the easier side, into registers */
		/* left then right, max(sul,sur+nr) */
		/* right then left, max(sur,sul+nr) */
holdit();
		/* to hold both sides in regs: nr+nr */
		nr = szty( p->left->type );
		sul = zum( p->left, ZLONG|ZFLOAT );
		sur = zum( p->right, ZLONG|ZFLOAT );
		p->su = min( max(sul,sur+nr), max(sur,sul+nr) );
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
			if( p->type!=FLOAT && p->type!=DOUBLE ) nr *= 2;
*/
			goto gencase;

		case ASG PLUS:
		case ASG MINUS:
		case ASG AND:
		case ASG OR:
			if( p->type == INT || p->type == UNSIGNED
			    || ISPTR(p->type) ) goto asop;

		gencase:
		default:
			sur = zum( p->right, ZLONG|ZFLOAT );
			if( sur == 0 ){ /* easy case: if addressable,
				do left value, op, store */
				if( sul == 0 ) p->su = nr;
				/* harder: left adr, val, op, store */
				else p->su = max( sul, nr+1 );
				}
			else { /* do right, left adr, left value, op, store */
				if( sul == 0 ){  /* right, left value, op, store */
					p->su = max( sur, nr+nr );
					}
				else {
					if (p->type == FLOAT
					    || p->type == DOUBLE)
					p->su = max(sur, max( sul, 1+nr));
					else
					p->su = max( sur, max( sul+nr, 1+nr+nr ) );
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
		p->su = max( max(sul,sur), nr);
		return;
		}

	if( ( o==DIV || o==MOD || o==MUL )
	    && p->type!=FLOAT && p->type!=DOUBLE ) nr *= 2;
	if( o==PLUS || o==MUL || o==AND || o==OR || o==ER ){
		/* permute: get the harder on the left */

		register rt, lt;

		if( istnode( p->left ) || sul > sur ) goto noswap;  /* don't do it! */

		/* look for a funny type on the left, one on the right */


		lt = p->left->type;
		rt = p->right->type;

		if( rt == FLOAT && lt == DOUBLE ) goto swap;

		if( (rt==CHAR||rt==UCHAR)
		    && (lt==INT||lt==UNSIGNED||ISPTR(lt)) ) goto swap;

		if( lt==LONG || lt==ULONG ){
			if( rt==LONG || rt==ULONG ){
				/* if one is a STARNM, swap */
				if( p->left->op == UNARY MUL && sul==0 )
					goto noswap;
				if( p->right->op == UNARY MUL
				    && p->left->op != UNARY MUL )
					goto swap;
				goto noswap;
				}
			else if( p->left->op == UNARY MUL && sul == 0 )
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
			s = p->left;  p->left = p->right; p->right = s;
			}
		}
	noswap:

	sur = zum( p->right, ZLONG|ZFLOAT );
	if( (sur == 0) && p->type !=FLOAT && p->type != DOUBLE) {
		/* get left value into a register, do op */
		p->su = max( nr, sul );
		}
	else {
		/* do harder into a register, then easier */
		p->su = max( nr+nr, min( max( sul, nr+sur ), max( sur, nr+sul ) ) );
		}
	}

holdit() {;}
int radebug = 0;

mkrall( p, r ) register NODE *p; {
	/* insure that the use of p gets done with register r; in effect, */
	/* simulate offstar */

	if( p->op == FLD ){
		p->left->rall = p->rall;
		p = p->left;
		}

	if( p->op != UNARY MUL ) return;  /* no more to do */
	p = p->left;
	if( p->op == UNARY MUL ){
		p->rall = r;
		p = p->left;
		}
	if( p->op == PLUS && p->right->op == ICON ){
		p->rall = r;
		p = p->left;
		}
	rallo( p, r );
	}

rallo( p, down ) register NODE *p; {
	/* do register allocation */
	register o, type, down1, down2, ty;

	if( radebug ) printf( "rallo( %o, %o )\n", p, down );

	down2 = NOPREF;
	p->rall = down;
	down1 = ( down &= ~MUSTDO );

	ty = optype( o = p->op );
	type = p->type;


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
		return;
		p->left->rall = R5|MUSTDO;
		mkrall( p->left, R4|MUSTDO );
		rallo( p->right, NOPREF );
		return;
/*
		if ((type == LONG) || (type == ULONG)) return;
*/
		/* keep the addresses out of the hair of (r0,r1) */
		if(fregs == 2 ){
			/* lhs in (r0,r1), nothing else matters */
			down1 = R1|MUSTDO;
			down2 = NOPREF;
			break;
			}
		/* at least 3 regs free */
		/* compute lhs in (r0,r1), address of left in r2 */
		p->left->rall = R1|MUSTDO;
		mkrall( p->left, R2|MUSTDO );
		/* now, deal with right */
		if( fregs >= 3 ) rallo( p->right, NOPREF );
		else {
			/* put address of long or value here */
			p->right->rall = R3|MUSTDO;
			mkrall( p->right, R3|MUSTDO );
			}
		return;

	case MUL:
	case DIV:
	case MOD:
		return;
		rallo( p->left, R4|MUSTDO );
		p->right->rall = R5|MUSTDO;
		mkrall( p->right, R6|MUSTDO );
		return;
/*
		if ((type == LONG) || (type == ULONG)) return;
*/
		rallo( p->left, R1|MUSTDO );

		if( fregs == 2 ){
			rallo( p->right, NOPREF );
			return;
			}
		/* compute addresses, stay away from (r0,r1) */

		p->right->rall = (fregs==3) ? R2|MUSTDO : R3|MUSTDO ;
		mkrall( p->right, R2|MUSTDO );
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

	if( ty != LTYPE ) rallo( p->left, down1 );
	if( ty == BITYPE ) rallo( p->right, down2 );

	}

offstar( p ) register NODE *p; {
	/* handle indirections */

	if( p->op == UNARY MUL ) p = p->left;

	if( p->op == PLUS || p->op == MINUS ){
		if( p->right->op == ICON ){
			order( p->left , INTAREG|INAREG );
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

	return( p->op == UNARY MUL && /* (t=p->type)!=CHAR && */
/*
		t!= UCHAR && t!= FLOAT &&
CCF for now */
/*		t != UCHAR && */
		shumul( p->left) != STARREG );
	}

setbin( p ) register NODE *p; {
	register NODE *r, *l;

	if(odebug) printf("setbin(%o)\n",p);
	r = p->right;
	l = p->left;
if (rdebug) {
	printf("setbin(%o), l->su(%d), r->su(%d)\n",p,l->su,r->su);
}

if(l->type != DOUBLE && l->type != FLOAT)
	if( p->right->su == 0 ){ /* rhs is addressable */
		if( logop( p->op ) ){
			if( l->op == UNARY MUL && l->type != DOUBLE
			    && l->type!=FLOAT && shumul( l->left ) != STARREG )
				offstar( l->left );
			else order( l, INTAREG|INAREG|INTBREG|INBREG|INTEMP );
			return( 1 );
		}
		if( !istnode( l ) ){
			if( niceuty( l ) )
				offstar( l->left );
			else
				order( l, INTAREG|INTBREG );
			return( 1 );
		}
		/* rewrite */
		return( 0 );
	}
	/* now, rhs is complicated: must do both sides into registers */
	/* do the harder side first */

	if( logop( p->op ) ){
		/* relational: do both sides into regs if need be */

		if( r->su > l->su ){
			if( niceuty(r) ){
				offstar( r->left );
				return( 1 );
			}
			else if( !istnode( r ) ){
				order( r, INTAREG|INAREG|INTBREG|INBREG|INTEMP);
				return( 1 );
			}
		}
		if( niceuty(l) ){
			offstar( l->left );
			return( 1 );
		}
/*
		else if( niceuty(r) ){
			offstar( r->left );
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
		cerror( "setbin can't deal with %s", opst[p->op] );
	}

	/* ordinary operator */

	if( !istnode(r) && r->su > l->su ){
		/* if there is a chance of making it addressable, try it... */
		if( niceuty(r) ){
			offstar( r->left );
			return( 1 );  /* hopefully, it is addressable by now */
		}
		order( r, INTAREG|INAREG|INTBREG|INBREG|INTEMP );  /* anything goes on rhs */
		return( 1 );
	}
	else {
		if( !istnode( l ) ){
			if(( l->type == FLOAT || l->type == DOUBLE )
			    && niceuty(l)) {
				offstar( l->left );
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
	if( p->right->op != REG ){
		order( p->right, INTAREG );
		return(1);
		}
	p = p->left;
	if( p->op != NAME && p->op != OREG ){
		if( p->op != UNARY MUL ) cerror( "bad setstr" );
		order( p->left, INTAREG );
		return( 1 );
		}
	return( 0 );
	}

setasg( p ) register NODE *p; {
	/* setup for assignment operator */

	if(odebug) printf("setasg(%o)\n",p);
	if( p->right->su != 0 && p->right->op != REG ) {
		if( p->right->op == UNARY MUL )
			offstar( p->right->left );
		else
			order( p->right, INTAREG|INAREG|INTBREG|INBREG|SOREG|SNAME|SCON );
		return(1);
		}
	if( p->right->op != REG && ( p->type == FLOAT || p->type == DOUBLE ) ) {
		order( p->right, INTAREG );
		return(1);
		}
	if( p->left->op == UNARY MUL && !tshape( p->left, STARREG|STARNM ) ){
		offstar( p->left->left );
		return(1);
		}
	if( p->left->op == FLD && p->left->left->op == UNARY MUL ){
		offstar( p->left->left->left );
		return(1);
		}
	/* if things are really strange, get rhs into a register */
	if( p->right->op != REG ){
		order( p->right, INTAREG|INAREG|INTBREG|INBREG );
		return( 1 );
		}
	return(0);
	}

setasop( p ) register NODE *p; {
	/* setup for =ops */
	register sul, sur;
	register NODE *q, *p2;

	if(odebug) printf("setasop(%o)\n",p);
	sul = p->left->su;
	sur = p->right->su;

	/*
	switch( p->op ){

	case ASG PLUS:
	case ASG OR:
	case ASG MINUS:
		if( p->type != INT && p->type != UNSIGNED
		    && !ISPTR(p->type) ) break;
		if( p->right->type == CHAR || p->right->type == UCHAR ){
			order( p->right, INTAREG|INAREG );
			return( 1 );
		}
	}
	*/

	if( sur == 0 ){

	if( ( p->op == ASG LS || p->op == ASG RS )
	    && p->right->op != ICON && !istnode( p->right ) ) {
		order( p->right, INTAREG );
		return( 1 );
	}
	leftadr:
		/* easy case: if addressable, do left value, op, store */
		if( sul == 0 ) goto rew;  /* rewrite */

		/* harder; make aleft address, val, op, and store */

		/*ERR*/
		if( (p->type == FLOAT || p->type == DOUBLE)
		     && p->left->op == UNARY MUL )
			return( 0 );
		if( p->left->op == UNARY MUL ){
			offstar( p->left->left );
			return( 1 );
		}
		if( p->left->op == FLD && p->left->left->op == UNARY MUL ){
			offstar( p->left->left->left );
			return( 1 );
		}
	rew:	/* for Z8000, autoincrement and autodecrement are not */
		/*   allowed, so we will return here */

		return(0);
	}

	/* harder case: do right, left address, left value, op, store */

	if( p->right->op == UNARY MUL ){
		offstar( p->right->left );
		return( 1 );
	}
	/* sur> 0, since otherwise, done above */
	if( p->right->op == REG ) goto leftadr;  /* make lhs addressable */
	order( p->right, INTAREG|INAREG|INTBREG|INBREG );
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

	/* generate code for the arguments */
	if(odebug) printf( "genargs(%o)\n",p );

	/*  first, do the arguments on the right (last->first) */
	while( p->op == CM ){
		genargs( p->right );
		p->op = FREE;
		p = p->left;
	}

	if( p->op == STARG ){ /* structure valued argument */
#define PUSHSZ 8

		size = p->stsize;
		if( p->left->op == ICON ){
			/* make into a name node */
			p->left->op == NAME;
			/* if( size <= PUSHSZ ) { */
				p->op = FREE;
				p = p->left;
			/* } */
		}
		else {
			/* make it look beautiful... */
			p->op = UNARY MUL;
			canon( p );  /* turn it into an oreg */
			if( p->op != OREG ){
				offstar( p->left );
				canon( p );
				if( p->op != OREG ) cerror( "stuck starg" );
			}
		}

		/* if( size <= PUSHSZ ) { */
			p->lval += size;  /* end of structure */
			/* put on stack backwards */
			if( size & 2 ) { /* odd number of words */
				p->lval -= 2;
				expand( p, RNOP, "#$\tpush\t*r15,ARZ-#)\n" );
				size -= 2;
			}
			for( ; size>0; size -= 4 ){
				p->lval -= 4;
				expand( p, RNOP, "#$\tpushl\t*r15,ARZ-Z-#)\n" );
			}
			reclaim( p, RNULL, 0 );
			return;
		/* } */
	}

	/* ordinary case */

	order( p, FORARG );
}

argsize( p ) register NODE *p; {
	register t;
	t = 0;
	if( p->op == CM ){
		t = argsize( p->left );
		p = p->right;
		}
	if( p->type == DOUBLE || p->type == FLOAT ){
		SETOFF( t, 2 );
		return( t+8 );
		}
	else if( p->type == LONG || p->type == ULONG ) {
		SETOFF( t, 2);
		return( t+4 );
		}
	else if( p->op == STARG ){
		SETOFF( t, p->stalign );  /* alignment */
		return( t + p->stsize );  /* size */
		}
	else {
		SETOFF( t, 2 );
		return( t+2 );
		}
	}
