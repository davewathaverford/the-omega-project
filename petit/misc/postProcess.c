#include <stdio.h>
#include <string.h>
#include <assert.h>


/*****************************************
 *                                       *
 *              WARNING                  *
 *                                       *
 *   The format fields in the structs    *
 *   below must end in a format (eg %s)  *
 *   to force scanf to match the entire  *
 *   pattern and return the expected     *
 *   result.                             *
 *                                       *
 *****************************************/

#define NELEM(ARRAY)   ( sizeof(ARRAY)/sizeof((ARRAY)[0]) )


typedef enum { omega1 = 0, omega2 = 1,
		   kill = 2, refine = 3, zap = 4,
		   junk = 5 } times;


int t_flag = 1;

char option_id[] = "12krz";  /* letters that can be used to specify output */
struct {
    int nfields;
    int fields[junk];
} outputs[1024];
int Noutputs = 0;

int add_output(char *spec)
{
    if (Noutputs < NELEM(outputs))
    {
	int i;
	for (i=0; i<strlen(spec); i++)
	{
	    if (strchr(option_id, spec[i]))
		outputs[Noutputs].fields[outputs[Noutputs].nfields++] =
		    strchr(option_id, spec[i]) - option_id;
	    else
		return 0;
	}
    }
    Noutputs++;
    return 1;
}

void add_default_outputs()
{
    for (Noutputs = 0; Noutputs < junk; Noutputs++)
    {
	outputs[Noutputs].nfields = 1;
	outputs[Noutputs].fields[0] = Noutputs;
    }
}

char *output_title(int o)
{
    int i;
    static char result[sizeof(option_id)], *r;

    assert(o < Noutputs);

    r = result;
    for (i=0; i<outputs[o].nfields; i++)
    {
	*r++ = option_id[outputs[o].fields[i]];
    }
    *r = 0;
    return result;
}



typedef struct {
    long a1, a2;  /* unordered pair of accesses */
/*    enum { output, flow } kind; */
    float t[junk];
} time_rec;
time_rec time_recs[65536];

struct {
    char format[128];
    int  ninputs;
    int  field1, field2;
} inputs[] = {
{ "O %lx %lx %f %f", 4, omega1, omega2 },
{ "F %lx %lx %f %f", 4, omega1, omega2 },
{ "K %lx %lx %f", 3, kill, junk},
{ "R %lx %lx %f", 3, refine, junk },
{ "Z %lx %lx %f", 3, zap, junk },
};


/* return index of pair (a1, a2), or an empty slot */
int findrec(a1, a2)
long a1, a2;
{
    int i;

    assert(a1 <= a2);

    for (i=0; i<NELEM(time_recs);i++)
    {
	assert((!time_recs[i].a1 && !time_recs[i].a2) ||
	       (time_recs[i].a1 && time_recs[i].a2));

	if ((a1 == time_recs[i].a1 && a2 == time_recs[i].a2) ||
	    (0 == time_recs[i].a1 && 0 == time_recs[i].a2))
	    return i;
    }

    assert(0 && "out of storage");
}

float output_value(int i, int o)
{
    float result;
    int f;
    result = 0;
    for (f=0; f<outputs[o].nfields; f++)
    {
	result += time_recs[i].t[outputs[o].fields[f]];
    }
    return result;
}



void noop(char *ignored)
{
}

/* when we hit a new file, pointers are invalidated -
   dump out data so far and reset */

void newfile(char *buf)
{
    if (time_recs[0].a1 && time_recs[0].a2)
    {
	int i, o;
	float  total[NELEM(outputs)];
	static time_rec nulltr;

	printf("# ");
	for (o=0; o<Noutputs; o++)
	{
	    printf("%7s ", output_title(o));
	    total[o] = 0;
	}
	printf("\n");
	
	for (i=0; time_recs[i].a1 && time_recs[i].a2; i++) {
	    assert(time_recs[i].t[omega1] >= 0 && time_recs[i].t[omega2] >= 0);

	    if(t_flag)
	      printf("  ");
	    for (o=0; o<Noutputs; o++)
	    {
		float tmp;
		tmp = output_value(i,o);
		total[o] += tmp;
		if(t_flag)
		  printf("%7f ", tmp);
	    }

	    if(t_flag)
	      printf("# %lx %lx\n", time_recs[i].a1, time_recs[i].a2);

	    time_recs[i] = nulltr;
	}

	printf("# ");
	for (o=0; o<Noutputs; o++)
	{
	    printf("%7f ", total[o]);
	}
	printf(" total\n\n");
    }
    
    printf("# %s", buf);
}

struct {
    char format[128];
    int  ninputs;
    void (*func)(char *);
} specials[] = {
    { "Parsing %s", 1, newfile } ,
    { "K from%s", 1, noop },
    { "K %lx %lx Didn't try accurate %s", 3, noop },
    { "K %lx %lx Quick %s", 3, noop },
    { "Z from%s", 1, noop },
    { "R from%s", 1, noop },
    { "O from%s", 1, noop },
    { "F from%s", 1, noop },
    { "WARNING%s", 1, noop }
};

main(int argc, char *argv[])
{
    FILE *source_file;
    char buf[1024];

    int c, errflag = 0;
    extern char *optarg;
    extern int optind;

    while ((c = getopt(argc, argv, "ts:")) != -1)
    {
	switch (c) {
	case 't':
	  t_flag = 0;
	  break;
	case 's':
	    if (!add_output(optarg)) errflag++;
	    break;
	case '?':
	    errflag++;
	}
    }
    if (errflag) {
	fprintf(stderr,
		"usage: %s <-s[%s]+> <timing_trace_file>\n",
		argv[0], option_id);
	exit (2);
    }

    if (Noutputs == 0)
	add_default_outputs();

    if (optind == argc-1)  {
	if ( (source_file = fopen(argv[optind],"r")) == NULL ) {
	    printf("%s: unable to open file %s \n", argv[0], argv[optind]);
	    exit(1);
	}
    } else {
	source_file = stdin;
    }

    while ( fgets(buf, 1024, source_file) )
    {
	int i;
	long a1, a2;
	float f1, f2;

	if (strlen(buf) <= 1) continue;

	for (i=0; i<NELEM(inputs); i++)
	{
	    if (sscanf(buf, inputs[i].format, &a1, &a2, &f1, &f2) ==
		inputs[i].ninputs)
	    {
		int which;
		if (a1 > a2)
		{
		    long tmp;
		    tmp = a1; a1 = a2; a2 = tmp;
		}
		which = findrec(a1, a2);
		assert((time_recs[which].a1!=0 && time_recs[which].a2!=0) ||
		       (time_recs[which].t[0] == 0 &&
			time_recs[which].t[1] == 0 &&
			time_recs[which].t[2] == 0 &&
			time_recs[which].t[3] == 0 &&
			time_recs[which].t[4] == 0));

		assert((time_recs[which].a1==a1 && time_recs[which].a2==a2)
		       || (time_recs[which].a1==0 && time_recs[which].a2==0));

		time_recs[which].a1 = a1;
		time_recs[which].a2 = a2;

		if (inputs[i].field1 != junk)
		    time_recs[which].t[inputs[i].field1] += f1;
		if (inputs[i].field2 != junk)
		    time_recs[which].t[inputs[i].field2] += f2;
		break;
	    }
	}
	if (i == NELEM(inputs)) {  /* no match */
	    char j[1024];  /* throw away inputs */
	    for (i=0; i<NELEM(specials); i++)
	    {
		assert(specials[i].ninputs <= 5);
		if (sscanf(buf, specials[i].format, &j, &j, &j, &j, &j) ==
		    specials[i].ninputs)
		{
		    (*specials[i].func)(buf);
		    break;
		}
	    }
	    if (i == NELEM(specials))
		fprintf(stderr, "%s: unmatched line: %s", argv[0], buf);
	}
    }

    newfile("thats all folks\n");

    if (argc == 2)
	fclose(source_file);
}
