#
/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include	"defs.h"
#include	<errno.h>
#include	"sym.h"
#ifdef STOCKIII
#else
#include	<sys/types.h>
#include	<sys/times.h>
#endif

LOCAL INT	parent;


SYSTAB		commands;
SYSTAB		xcmds;



/* ========	command execution	========*/


execute(argt, execflg, pf1, pf2)
	TREPTR		argt;
	INT		*pf1, *pf2;

{
	/* `stakbot' is preserved by this routine */
	REG TREPTR	t;
	STKPTR		sav=savstak();
	INT	bltflg=0;

	sigchk();

	IF (t=argt) ANDF execbrk==0
	THEN	REG INT		treeflgs;
		INT		oldexit, type;
		REG STRING	*com;

		treeflgs = t->tretyp; type = treeflgs&COMMSK;
		oldexit=exitval; exitval=0;

		SWITCH type IN

		case TCOM:
			BEGIN
			STRING		a1;
			INT		argn, internal;
			ARGPTR		schain=gchain;
			IOPTR		io=t->treio;
			gchain=0;
			argn = getarg(t);
			com=scan(argn);
			a1=com[1]; gchain=schain;

			IF (internal=syslook(com[0],commands)) ORF argn==0
			THEN	setlist(((COMPTR) t)->comset, 0);
			FI

			IF argn ANDF (flags&noexec)==0
			THEN	/* print command if execpr */
				IF flags&execpr
				THEN	argn=0;	prs(execpmsg);
					WHILE com[argn]!=ENDARGS
					DO prs(com[argn++]); blank() OD
					newline();
				FI


				SWITCH internal IN

				case SYSDOT:

					IF a1
					THEN	REG INT		f;
	
						IF (f=pathopen(getpath(a1), a1)) < 0
						THEN failed(a1,notfound);
						ELSE execexp(0,f);
						FI
					FI
					break;

				case SYSTIMES:
					{
					L_INT	t[4]; times(t);
#ifdef STOCKII
					prt(t[2]); blank(); prt(t[3]); newline();
#else
					prt(cv50to60(t[2])); blank();
					prt(cv50to60(t[3])); newline();
#endif
					}
					break;
	
				case SYSEXIT:

					flags |= forked; /* force exit */
					exitsh(a1?stoi(a1):oldexit);
	
				case SYSNULL:

					io=0;
					break;
	
				case SYSCONT:

					IF (execbrk = -loopcnt) ANDF a1
					THEN	breakcnt = stoi(a1);
					FI
					break;
	
				case SYSBREAK:

					IF (execbrk=loopcnt) ANDF a1
					THEN breakcnt=stoi(a1);
					FI
					break;
	
				case SYSTRAP:

					IF a1
					THEN	BOOL	clear;
						IF (clear=digit(*a1))==0
						THEN	++com;
						FI
						WHILE *++com
						DO INT	i;
						   IF (i=stoi(*com))>=MAXTRAP ORF i<MINTRAP
						   THEN	failed(*com,badtrap);
						   ELIF clear 
						   THEN	clrsig(i);
						   ELSE	replace(&trapcom[i],a1);
							IF *a1
							THEN	getsig(i);
							ELSE	ignsig(i);
							FI
						   FI
						OD
					ELSE	/* print out current traps */
						INT		i;
	
						FOR i=0; i<MAXTRAP; i++
						DO IF trapcom[i] 
						   THEN	prn(i); prs(colon); prs(trapcom[i]); newline();
						   FI
						OD
					FI
					break;
	
				case SYSEXEC:

					com++;
					initio(io); ioset=0; io=0;
					IF a1==0 THEN break FI

#ifdef RES /*	Research includes login as part of shell	*/
				case SYSLOGIN:

					oldsigs(); execa(com); done();
#else
				case SYSNEWGRP:

					IF flags&rshflg
					THEN
						failed(com[0],restricted);
					ELSE
						flags |= forked; /* force bad exec to terminate shell */
						oldsigs(); execa(com); done();
					FI

#endif
				case SYSCD:

					IF flags&rshflg
					THEN	failed(com[0],restricted);
					ELIF	(argn >2) THEN failed(com[0], "argument count");
					ELIF (a1==0 ANDF (a1=homenod.namval)==0) ORF chdir(a1)<0
					THEN	failed(a1,baddir);
					FI
					break;
	
	
				case SYSSHFT:
					IF dolc<1
					THEN	error(badshift);
					ELSE	dolv++; dolc--;
					FI
					assnum(&dolladr, dolc);
					break;
	
				case SYSWAIT:

					/*
					await(-1);
					*/
					await(a1?stoi(a1):-1,1);
					break;
	
				case SYSREAD:

					rwait=1;
					exitval=readvar(&com[1]);
					rwait=0;
					break;

				case SYSSET:

					IF a1
					THEN	INT	argc;
						argc = options(argn,com);
						IF argc>1
						THEN setargs(com+argn-argc);
						FI
					ELIF ((COMPTR) t)->comset==0
					THEN	/*scan name chain and print*/
						namscan(printnam);
					FI
					break;
	
				case SYSRDONLY:

					exitval=N_RDONLY;
				case SYSXPORT:

					IF exitval==0 THEN exitval=N_EXPORT; FI
	
					IF a1
					THEN	WHILE *++com
						DO attrib(lookup(*com), exitval) OD
					ELSE	namscan(printflg);
					FI
					exitval=0;
					break;
	
				case SYSEVAL:

					IF a1
					THEN	execexp(a1,&com[2]);
					FI
					break;
				case SYSUMASK:

					IF a1
					THEN 	INT c, i;
						i = 0;
						WHILE (c = *a1++) >= '0' && c<= '7'
						DO	i= (i << 3) + c - '0'
						OD
						umask(i);
					ELSE
						INT i, j;
						umask(i= umask(0));
						prc('0');
						FOR j=6;j>=0;j-=3
						DO	prc(((i>>j)&07) +'0');
						OD
							newline();
					FI
					break;
				default:

					IF (internal=syslook(com[0],xcmds)) >0
					THEN	builtin(internal, argn, com);
						bltflg=1;
					FI
	
				ENDSW

				IF internal
				THEN	IF io ANDF !bltflg THEN error(illegal) FI
					chktrap();
					break;
				FI
			ELIF t->treio==0
			THEN	chktrap();
				break;
			FI
			END
		case TFORK:
			IF execflg ANDF (treeflgs&(FAMP|FPOU))==0
			THEN	parent=0;
			ELSE	BEGIN
					/* FORKLIM is the max period between forks -
					   power of 2 usually.  Currently shell tries after
					   2,4,8,16, and 32 seconds and then quits */
					INT forkcnt=1;
					WHILE (parent=fork()) == -1
					DO 	IF (forkcnt=(forkcnt*2)) > FORKLIM  /* 32 */
						THEN SWITCH errno IN
							case ENOMEM:	error(noswap);
									break;
							default:
							case EAGAIN: error(nofork);
									break;
						     ENDSW
						FI
						sigchk(); alarm(forkcnt);
						pause(); 
					OD
				END
			FI

			IF parent
			THEN	/* This is the parent branch of fork;    */
				/* it may or may not wait for the child. */
				IF treeflgs&FPRS ANDF flags&ttyflg
				THEN	prn(parent); newline();
				FI
				IF treeflgs&FPCL THEN closepipe(pf1) FI
				IF (treeflgs&(FAMP|FPOU))==0
				THEN	await(parent,0);
				ELIF (treeflgs&FAMP)==0
				THEN	post(parent);
				ELSE	assnum(&pcsadr, parent);
				FI

				chktrap();
				break;


			ELSE	/* this is the forked branch (child) of execute */
				flags |= forked; iotemp=0;
				postclr();
				settmp();

				/* Turn off INTR and QUIT if `FINT'  */
				/* Reset ramaining signals to parent */
				/* except for those `lost' by trap   */
				oldsigs();
				IF treeflgs&FINT
				THEN	signal(INTR,1); signal(QUIT,1);
				FI

				/* pipe in or out */
				IF treeflgs&FPIN
				THEN	rename(pf1[INPIPE],0);
					close(pf1[OTPIPE]);
				FI
				IF treeflgs&FPOU
				THEN	rename(pf2[OTPIPE],1);
					close(pf2[INPIPE]);
				FI

				/* default std input for & */
				IF treeflgs&FINT ANDF ioset==0
				THEN	rename(chkopen(devnull),0);
				FI

				/* io redirection */
				initio(t->treio);
				IF type!=TCOM
				THEN	execute(((FORKPTR) t)->forktre,1);
				ELIF com[0]!=ENDARGS
				THEN	setlist(((COMPTR) t)->comset,N_EXPORT);
					execa(com);
				FI
				done();
			FI

		case TPAR:
			rename(dup(2),output);
			execute(((PARPTR) t)->partre,execflg);
			done();

		case TFIL:
			BEGIN
			   INT pv[2]; chkpipe(pv);
			   IF execute(((LSTPTR) t)->lstlef, 0, pf1, pv)==0
			   THEN	execute(((LSTPTR) t)->lstrit, execflg, pv, pf2);
			   ELSE	closepipe(pv);
			   FI
			END
			break;

		case TLST:
			execute(((LSTPTR) t)->lstlef,0);
			execute(((LSTPTR) t)->lstrit,execflg);
			break;

		case TAND:
			IF execute(((LSTPTR) t)->lstlef,0)==0
			THEN	execute(((LSTPTR) t)->lstrit,execflg);
			FI
			break;

		case TORF:
			IF execute(((LSTPTR) t)->lstlef,0)!=0
			THEN	execute(((LSTPTR) t)->lstrit,execflg);
			FI
			break;

		case TFOR:
			BEGIN
			   NAMPTR	n = lookup(((FORPTR) t)->fornam);
			   STRING	*args;
			   DOLPTR	argsav=0;

			   IF ((FORPTR) t)->forlst==0
			   THEN    args=dolv+1;
				   argsav=useargs();
			   ELSE	   ARGPTR	schain=gchain;
				   gchain=0;
				   trim((args=scan(getarg(((FORPTR) t)->forlst)))[0]);
				   gchain=schain;
			   FI
			   loopcnt++;
			WHILE *args != ENDARGS ANDF execbrk <= 0
			DO
				assign(n, *args++);
				execute(((FORPTR) t)->fortre,0);
				IF execbrk
				THEN IF breakcnt > 1 ORF execbrk > 0
				      THEN break;
				     ELSE execbrk = breakcnt = 0;
				     FI
				FI
			   OD
			   IF breakcnt THEN breakcnt-- FI
			   execbrk = (execbrk < 0 ? -breakcnt : breakcnt);
			   loopcnt--;
			   argfor=(DOLPTR)freeargs(argsav);
			END
			break;

		case TWH:
		case TUN:
			BEGIN
			   INT		i=0;

			   loopcnt++;
			   WHILE execbrk<=0 ANDF (execute(((WHPTR) t)->whtre,0)==0)==(type==TWH)
			   DO
				i = execute(((WHPTR) t)->dotre, 0);
				IF execbrk
				THEN	IF breakcnt > 1 ORF execbrk > 0
					THEN break;
					ELSE	execbrk = breakcnt = 0;
					FI
				FI
			   OD
			   IF breakcnt THEN breakcnt-- FI
			   execbrk=(execbrk < 0 ? -breakcnt : breakcnt);
			   loopcnt--; exitval= i;
			END
			break;

		case TIF:
			IF execute(((IFPTR) t)->iftre,0)==0
			THEN	execute(((IFPTR) t)->thtre,execflg);
			ELIF	((IFPTR) t)->eltre
			THEN	execute(((IFPTR) t)->eltre, execflg);
			ELSE 	exitval=0; /* force zero exit for if-then-fi */
			FI
			break;

		case TSW:
			BEGIN
			   REG STRING	r = mactrim(((SWPTR) t)->swarg);
			   t=(TREPTR)((SWPTR) t)->swlst;
			   WHILE t
			   DO	ARGPTR		rex=(ARGPTR)((REGPTR) t)->regptr;
				WHILE rex
				DO	REG STRING	s;
					IF gmatch(r,s=macro(((ARGPTR) rex)->argval)) ORF (trim(s), eq(r,s))
					THEN	execute(((REGPTR) t)->regcom,0);
						t=0; break;
					ELSE	rex=rex->argnxt;
					FI
				OD
				IF t THEN t=(TREPTR)((REGPTR) t)->regnxt FI
			   OD
			END
			break;
		ENDSW
		exitset();
	FI

	sigchk();
	tdystak(sav);
	return(exitval);
}


execexp(s,f)
	STRING		s;
	UFD		f;
{
	FILEBLK		fb;
	push(&fb);
	IF s
	THEN	estabf(s); fb.feval=(STRING *)(f);
	ELIF f>=0
	THEN	initf(f);
	FI
	execute(cmd(NL, NLFLG|MTFLG),0);
	pop();
}
