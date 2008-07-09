#ifndef Already_included_hpp
#define Already_included_hpp

namespace omega {

typedef enum {block, cyclic, star} dist_type;

class dist_info {
public: 
    dist_info(dist_type _type, int _blocksize=0):
	type(_type), blocksize(_blocksize){}
    dist_info(): type(star), blocksize(-1){}//cause an assertion fail if you use
    bool operator==(const dist_info &d) {return(type == d.type && blocksize==d.blocksize);}
    bool operator!=(const dist_info &d) {return(type != d.type || blocksize!=d.blocksize);}

    dist_type type;
    int blocksize;
};

typedef List<dist_info>  distribution_list;

class template_info {
public:
    template_info(String n, List<int> *l) : bounds_list(l), name(n) {}
    List<int> *bounds_list;
#if 0
    Relation distribution;
#endif
    distribution_list *dist_list;
    String name, proc_name;
};


void* build_hpp();
int hpp_generate(int);

Relation make_bounds_relation(List<int> *l);
symtabentry *lookup_symbol(const String &name);
List<int> *lookup_proc(const String &name);
List<int> *add_proc(const String &name, List<int> *l);
template_info *lookup_template(const String &name);
template_info *add_template(const String &name, List<int> *l);

extern FILE *hpp_debug_file;
}


#endif
