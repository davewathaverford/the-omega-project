/* $Id: depend_dir.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */
#ifndef Already_Included_Depend_Dir
#define Already_Included_Depend_Dir

#include <omega.h>
#include <basic/Dynamic_Array.h>

namespace omega {

#define maxnest 20
#define maxVars 56

typedef long unsigned int ddudirection;

#define ddufwd           (ddudirection)0x1
#define dduind           (ddudirection)0x2
#define ddubck           (ddudirection)0x4
#define dduall           (ddudirection)0x7

#define ddudirBits       (ddudirection)0x7777777

#define dduextract1(dv,n)        (((dv)>>(((n)-1)*4))&dduall)
#define ddutest1(dv,d)           ((dv)&(d))
#define ddudirnest(d,n)          ((d)<<(((n)-1)*4))
#define ddudirsetonly(dv,d,n)    (dv=(((dv)&~ddudirnest(dduall,n))|(ddudirnest(d,n))))

typedef struct dir_list
    {
    ddudirection dir;
    int nest, dim;
    struct dir_list *next;
    int differenceKnown[maxnest];
    int difference[maxnest];
    } dir_list;

typedef struct extended_dir_list
    {
    int dim;
    int pdim; // for reductions -- should #processors be included in volume?
    int zero[maxnest][maxnest];
    int constant[maxnest][maxnest];
    int size[maxnest][maxnest];
    int coefs[maxnest][maxnest][maxVars];
    int nest1, nest2;
    struct extended_dir_list *next;
    } extended_dir_list;

extern Tuple<Global_Var_ID> uniform_global_list;

extern ddudirection ddextract(dir_list *d, int n);
extern dir_list *add_direction(int stmt, unsigned int nest, NOT_CONST Relation &dep_rel);
extern void print_dir(dir_list *start);
extern void print_dirs(dir_list *start);
extern dir_list *compose_dir(dir_list *in, dir_list *out, dir_list **last);
extern int merged(dir_list *n, dir_list *m, dir_list *prevm);
extern void merge_dirs(dir_list *start, dir_list **last);
extern void close_dirs(Dynamic_Array2<dir_list *> &dir_array,int i,int k,int j);
extern void close_dir_array(Dynamic_Array2<dir_list *> &dir_array);
extern int is_plus(dir_list * n, int stmt, int j);
extern int is_minus(dir_list * n, int stmt, int j);
extern int contains_plus(dir_list * n, int stmt, int j);
extern int contains_minus(dir_list * n, int stmt, int j);
extern void add_dirs(Dynamic_Array2<dir_list *> &dir_array, bool reduce);

void compute_reduction_volumes();
void initialize_extended_dirs(Dynamic_Array2<extended_dir_list *> 
			      &extended_dir_array);
void add_extended_dirs(Dynamic_Array2<extended_dir_list *> &extended_dir_array);
extern void print_extended_dir(extended_dir_list *start);
extern void print_extended_dirs(extended_dir_list *start);
extern dir_list *dir_list_union(dir_list *d1, dir_list *d2);
extern void delete_dir_list(dir_list *tmp_list);

}

#endif
