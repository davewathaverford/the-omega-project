/**********************************************
 *					      *
 * Hacked to convert FORTRAN-77 to petit by    *
 * Vadim Maslov,  vadik@cs.umd.edu, 09/20/92. *
 *					      *
 **********************************************/

/****************************************************************
Copyright 1990, 1991 by AT&T Bell Laboratories and Bellcore.

Permission to use, copy, modify, and distribute this software
and its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appear in all
copies and that both that the copyright notice and this
permission notice and warranty disclaimer appear in supporting
documentation, and that the names of AT&T Bell Laboratories or
Bellcore or any of their entities not be used in advertising or
publicity pertaining to distribution of the software without
specific, written prior permission.

AT&T and Bellcore disclaim all warranties with regard to this
software, including all implied warranties of merchantability
and fitness.  In no event shall AT&T or Bellcore be liable for
any special, indirect or consequential damages or any damages
whatsoever resulting from loss of use, data or profits, whether
in an action of contract, negligence or other tortious action,
arising out of or in connection with the use or performance of
this software.
****************************************************************/

#include "defs.h"
#include "parse.h"

int complex_seen, dcomplex_seen;

LOCAL int Max_ftn_files;

char **ftn_files;
int current_ftn_file = 0;

flag ftn66flag = NO;
flag nowarnflag = NO;
flag noextflag = NO;
flag  no66flag = NO;		/* Must also set noextflag to this
					   same value */
flag zflag = YES;		/* recognize double complex intrinsics */
flag debugflag = NO;
flag onetripflag = NO;
flag shiftcase = YES;
flag undeftype = NO;
flag r8flag = NO;
flag use_bs = YES;
flag LabUnused = NO;		/* print unused labels as comments -- vadik */
int tyreal = TYREAL;
int tycomplex = TYCOMPLEX;
extern void r8fix();

int maxequiv = MAXEQUIV;
int maxext = MAXEXT;
int maxstno = MAXSTNO;
int maxctl = MAXCTL;
int maxhash = MAXHASH;
int maxliterals = MAXLITERALS;
int useauto=1;
int can_include = YES;	/* so we can disable includes for netlib */

static char *def_i2 = "";

static int useshortints = NO;	/* YES => tyint = TYSHORT */
static int uselongints = NO;	/* YES => tyint = TYLONG */
int addftnsrc = NO;		/* Include ftn source in output */
int forcedouble = NO;		/* force real functions to double */
int def_equivs = YES;
int tyioint = TYLONG;
int szleng = SZLENG;
int inqmask = M(TYLONG)|M(TYLOGICAL);
int wordalign = NO;
int forcereal = NO;
char *file_name, *filename0, *parens;
int chars_per_wd, gflag;
int infertypes = 1;
char used_rets[TYSUBR+1];
extern char *tmpdir;
static int h0align = 0;
char *halign, *ohalign;
int hsize;	/* for padding under -h */
int htype;	/* for wr_equiv_init under -h */

#define f2c_entry(swit,count,type,store,size) \
	p_entry ("-", swit, 0, count, type, store, size)

static arg_info table[] = {
    f2c_entry ("lu", P_NO_ARGS, P_INT, &LabUnused, YES),
    f2c_entry ("w66", P_NO_ARGS, P_INT, &ftn66flag, YES),
    f2c_entry ("w", P_NO_ARGS, P_INT, &nowarnflag, YES),
    f2c_entry ("66", P_NO_ARGS, P_INT, &no66flag, YES),
    f2c_entry ("d", P_ONE_ARG, P_INT, &debugflag, YES),
    f2c_entry ("1", P_NO_ARGS, P_INT, &onetripflag, YES),
    f2c_entry ("onetrip", P_NO_ARGS, P_INT, &onetripflag, YES),
    f2c_entry ("I2", P_NO_ARGS, P_INT, &useshortints, YES),
    f2c_entry ("I4", P_NO_ARGS, P_INT, &uselongints, YES),
    f2c_entry ("U", P_NO_ARGS, P_INT, &shiftcase, NO),
    f2c_entry ("u", P_NO_ARGS, P_INT, &undeftype, YES),
    f2c_entry ("Nq", P_ONE_ARG, P_INT, &maxequiv, 0),
    f2c_entry ("Nx", P_ONE_ARG, P_INT, &maxext, 0),
    f2c_entry ("Ns", P_ONE_ARG, P_INT, &maxstno, 0),
    f2c_entry ("Nc", P_ONE_ARG, P_INT, &maxctl, 0),
    f2c_entry ("Nn", P_ONE_ARG, P_INT, &maxhash, 0),
    f2c_entry ("NL", P_ONE_ARG, P_INT, &maxliterals, 0),
    f2c_entry ("c", P_NO_ARGS, P_INT, &addftnsrc, YES),
    f2c_entry ("R", P_NO_ARGS, P_INT, &forcedouble, NO),
    f2c_entry ("!R", P_NO_ARGS, P_INT, &forcedouble, YES),
    f2c_entry ("ext", P_NO_ARGS, P_INT, &noextflag, YES),
    f2c_entry ("z", P_NO_ARGS, P_INT, &zflag, NO),
    f2c_entry ("a", P_NO_ARGS, P_INT, &useauto, YES),
    f2c_entry ("r8", P_NO_ARGS, P_INT, &r8flag, YES),
    f2c_entry ("i2", P_NO_ARGS, P_INT, &tyioint, NO),
    f2c_entry ("w8", P_NO_ARGS, P_INT, &wordalign, YES),
    f2c_entry ("!I", P_NO_ARGS, P_INT, &can_include, NO),
    f2c_entry ("W", P_ONE_ARG, P_INT, &chars_per_wd, 0),
    f2c_entry ("g", P_NO_ARGS, P_INT, &gflag, YES),
    f2c_entry ("T", P_ONE_ARG, P_STRING, &tmpdir, 0),
    f2c_entry ("!it", P_NO_ARGS, P_INT, &infertypes, 0),
    f2c_entry ("h", P_NO_ARGS, P_INT, &h0align, 1),
    f2c_entry ("hd", P_NO_ARGS, P_INT, &h0align, 2),
    f2c_entry ("!bs", P_NO_ARGS, P_INT, &use_bs, NO),
    f2c_entry ("r", P_NO_ARGS, P_INT, &forcereal, YES),

	/* options omitted from man pages */

	/* -ev ==> implement equivalence with initialized pointers */
    f2c_entry ("ev", P_NO_ARGS, P_INT, &def_equivs, NO),

	/* -!it used to be the default when -it was more agressive */

    f2c_entry ("it", P_NO_ARGS, P_INT, &infertypes, 1)
}; /* table */

extern char *c_functions;	/* "c_functions"	*/
extern char *coutput;		/* "c_output"		*/
extern char *initfname;		/* "raw_data"		*/
extern char *blkdfname;		/* "block_data"		*/
extern char *p1_file;		/* "p1_file"		*/
extern char *p1_bakfile;	/* "p1_file.BAK"	*/
extern char *sortfname;		/* "init_file"		*/

extern void list_init_data(), set_tmp_names(), sigcatch(), Un_link_all();
extern char *c_name();


set_externs ()
{
    static char *hset[3] = { 0, "integer", "doublereal" };

/* Adjust the global flags according to the command line parameters */

    if (chars_per_wd > 0) {
	typesize[TYADDR] = typesize[TYLONG] = typesize[TYREAL] =
		typesize[TYLOGICAL] = chars_per_wd;
	typesize[TYDREAL] = typesize[TYCOMPLEX] = chars_per_wd << 1;
	typesize[TYDCOMPLEX] = chars_per_wd << 2;
	typesize[TYSHORT] = chars_per_wd >> 1;
	typesize[TYCILIST] = 5*chars_per_wd;
	typesize[TYICILIST] = 6*chars_per_wd;
	typesize[TYOLIST] = 9*chars_per_wd;
	typesize[TYCLLIST] = 3*chars_per_wd;
	typesize[TYALIST] = 2*chars_per_wd;
	typesize[TYINLIST] = 26*chars_per_wd;
	}

    if (wordalign)
	typealign[TYDREAL] = typealign[TYDCOMPLEX] = typealign[TYREAL];
    if (!tyioint) {
	tyioint = TYSHORT;
	szleng = typesize[TYSHORT];
	def_i2 = "#define f2c_i2 1\n";
	inqmask = M(TYSHORT)|M(TYLOGICAL);
	goto checklong;
	}
    else
	szleng = typesize[TYLONG];
    if (useshortints) {
	inqmask = M(TYLONG);
 checklong:
	typename[TYLOGICAL] = "shortlogical";
	typesize[TYLOGICAL] = typesize[TYSHORT];
	if (uselongints)
	    err ("Can't use both long and short ints");
	else
	    tyint = tylogical = TYSHORT;
	}
    else if (uselongints)
	tyint = TYLONG;

    if (h0align) {
	if (tyint == TYLONG && wordalign)
		h0align = 1;
    	ohalign = halign = hset[h0align];
	htype = h0align == 1 ? tyint : TYDREAL;
	hsize = typesize[htype];
	}

    if (no66flag)
	noextflag = no66flag;
    if (noextflag)
	zflag = 0;

    if (r8flag) {
	tyreal = TYDREAL;
	tycomplex = TYDCOMPLEX;
	r8fix();
	}

/* Check the list of input files */

    {
	int bad, i, cur_max = Max_ftn_files;

	for (i = bad = 0; i < cur_max && ftn_files[i]; i++)
	    if (ftn_files[i][0] == '-') {
		errstr ("Invalid flag '%s'", ftn_files[i]);
		bad++;
		}
	if (bad)
		exit(1);

    } /* block */
} /* set_externs */



 int retcode = 0;

main(argc, argv)
int argc;
char **argv;
{
	int k;
	FILE *c_output;
	char *cdfilename;
	static char stderrbuf[BUFSIZ];
	extern char link_msg[];

	diagfile = stderr;
	setbuf(stderr, stderrbuf);	/* arrange for fast error msgs */

	Max_ftn_files = argc - 1;
	ftn_files = (char **)ckalloc((argc+1)*sizeof(char *));

	parse_args (argc, argv, table, sizeof(table)/sizeof(arg_info),
		ftn_files, Max_ftn_files);

	parens = "()";
	
	set_externs();
	fileinit();

	for(k = 1; ftn_files[k]; k++)
		if (dofork())
			break;
	filename0 = file_name = ftn_files[current_ftn_file = k - 1];

	set_tmp_names();
	sigcatch();

	c_file   = opf(c_functions, textwrite);
	pass1_file=opf(p1_file, binwrite);
	initkey();
	if (file_name && *file_name) {
		if (debugflag != 1) {
			coutput = c_name(file_name,'t');
			}
		cdfilename = coutput;
		if (!(c_output = fopen(coutput, textwrite))) {
			file_name = coutput;
			coutput = 0;	/* don't delete read-only .t file */
			fatalstr("can't open %.86s", file_name);
			}
		}
	else {
		file_name = "";
		cdfilename = "f2p_out.c";
		c_output = stdout;
		coutput = 0;
		}

	if(inilex( copys(file_name) ))
		done(1);
	if (filename0) {
		fprintf(diagfile, "%s:\n", file_name);
		fflush(diagfile);
		}

	procinit();
	if(k = yyparse())
	{
		fprintf(diagfile, "Bad parse, return code %d\n", k);
		done(1);
	}

	if (nerr)
		goto C_skipped;


/* Write out the declarations which are global to this file */

	if (gflag)
		nice_printf (c_output, "#line 1 \"%s\"\n", file_name);
	nice_printf (c_file, "\n");
	fclose (c_file);
	c_file = c_output;		/* HACK to get the next indenting
					   to work */
	wr_common_decls (c_output);
	if (blkdfile)
		list_init_data(&blkdfile, blkdfname, c_output);
	wr_globals (c_output);
	if ((c_file = fopen (c_functions, textread)) == (FILE *) NULL)
	    Fatal("main - couldn't reopen c_functions");
	ffilecopy (c_file, c_output);
	/*
	if (*main_alias) {
	    nice_printf (c_output, "! Main program alias \n ");
	    nice_printf (c_output, "int %s () { MAIN__ (); }\n",
		    main_alias);
	    }
	*/
	fclose (c_output);

 C_skipped:
	if(parstate != OUTSIDE)
		{
		warn("missing final end statement");
		endproc();
		}
	done(nerr ? 1 : 0);
}


FILEP opf(fn, mode)
char *fn, *mode;
{
	FILEP fp;
	if( fp = fopen(fn, mode) )
		return(fp);

	fatalstr("cannot open intermediate file %s", fn);
	/* NOT REACHED */ return 0;
}


clf(p, what, quit)
 FILEP *p;
 char *what;
 int quit;
{
	if(p!=NULL && *p!=NULL && *p!=stdout)
	{
		if(ferror(*p)) {
			fprintf(stderr, "I/O error on %s\n", what);
			if (quit)
				done(3);
			retcode = 3;
			}
		fclose(*p);
	}
	*p = NULL;
}


done(k)
int k;
{
	clf(&initfile, "initfile", 0);
	clf(&c_file, "c_file", 0);
	clf(&pass1_file, "pass1_file", 0);
	Un_link_all(k);
	exit(k|retcode);
}
