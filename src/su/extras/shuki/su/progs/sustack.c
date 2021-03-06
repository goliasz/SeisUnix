/* sustack - stack adjacent traces with same key header word
 *
 * Credits:
 *	SEP: Einar
 *	CWP: Jack
 *
 * Caveat: Should make division by fold an option. SHOULD USE SUTRSEQ AND DO IT INCORE.
*/

#include <stdio.h>
#include "../include/su.h"

char *sdoc;
int xargc;
char **xargv;
bool verbose;
char *SccsId[]="@(#)sustack.c	1.3\t11/15/88\n";

/* bool hisout,bhout; */

/*********************** self documentation **********************/
char *sdoc = "\
								\n\
SUSTACK - sustack key=cdp <input >output		\n\
								\n\
";
/*****************************************************************/

main(argc, argv)
int argc; char **argv;
{
	Sutrace tri, tro;
	Subhed bh;
	char *key;	/* key header word			*/
	char *type;	/* ... its type				*/
	int index;	/* ... its index			*/
	int nt;		/* number of data points on trace	*/
	int nsegy;	/* number of bytes in the segy		*/
	value val;	/* value of key in current gather	*/
	value valnew;	/* value of key in trace being treated	*/
	int fold=1;	/* number of traces with same key value	*/
	int newtracl=1;	/* tracl for stacked traces		*/
	int j;
	int infd,outfd;
	int nbh;

	xargc = argc; xargv = argv;

	infd = input();
	outfd = output();

	nbh = sizeof(Sutrace) - sizeof(float*);

	/* GET OPTIONS */
	verbose = false;
	while( (j=getopt(xargc,xargv,"v"))!=EOF) {
		switch(j) {
		case 'v':
			verbose = true;
			break;
		case '?':
			warn(__FILE__,__LINE__,"getopt returned '?'");
		}
	}

	if(!sgetpar("key", &key)) key="cdp";
	type = hdtype(key);
	index = getindex(key);

	apass(infd,outfd);

	getbh(infd,&bh);
	nt = bh.ns;
	tri.data = (float*) malloc(bh.ns*bh.esize);
	tro.data = (float*) malloc(bh.ns*bh.esize);

	nsegy = gettr(infd,&tro);

	hislog(outfd);
/* 	hisclose(outfd); */

	putbh(outfd,&bh);

	gethval(&tro, index, &val);

	while (nsegy) {
		nsegy = gettr(infd,&tri);
		gethval(&tri, index, &valnew);
		if (valcmp(type, val, valnew) || /* new gather or ... */
		    !nsegy) {			 /* end of traces     */
			if (verbose) {
				fprintf(stderr, "val=");
				fprintfval(stderr, type, val);
				fprintf(stderr, "\tfold=%d\n", fold);
			}

			/* Add header info and output stack */
			tro.nhs = fold;
			tro.tracl = newtracl++;
		/*	if (fold != 1) vsmul(tro.data, 1.0/fold, nt); */
			puttr(outfd,&tro);

			/* Set up for next gather */
			bcopy(&tri, &tro, nbh);
			bcopy((char*)(tri.data), (char*)(tro.data), bh.ns*bh.esize);
			fold = 1;
			val = valnew;

		} else {	/* still in same gather */
			vadd(tro.data, tri.data, nt);
			fold++;
		}
	}
	exit(0);
}
