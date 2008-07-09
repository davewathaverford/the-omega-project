#if !defined(Already_Included_code_gen)
#define Already_Included_code_gen

#include <basic/bool.h>
#include <basic/Tuple.h>
#include <omega/Relation.h>
#include <code_gen/CG.h>
#include <code_gen/CG_outputRepr.h>
#include <code_gen/CG_outputBuilder.h>

namespace omega {

typedef Tuple<int> IntTuple;
typedef Tuple<Relation> SetTuple;
typedef Tuple<SetTuple> SetTupleTuple;
typedef Tuple<Relation> RelTuple;
typedef Tuple<RelTuple> RelTupleTuple;

extern int code_gen_check_zero_trip;

class naming_info;

Relation project_onto_levels(Relation R, int last_level, bool wildcards);

CG_outputRepr *
MMGenerateCode(CG_outputBuilder* ocg,
	       Tuple<Relation> &, Tuple<Relation> &, Relation &known,
	       int effort=0);

CG_outputRepr *
MMGenerateCode(CG_outputBuilder* ocg,
	       Tuple<Relation> &, Tuple<Relation> &,
	       Tuple<naming_info *> &,
	       Relation &known, int effort=0);

String MMGenerateCode(Tuple<Relation> &, Tuple<Relation> &, Relation &known,
			int effort=0);

String MMGeneratePythonCode(Tuple<Relation> &, Tuple<Relation> &, Relation &known,
			int effort=0);

String MMGenerateCode(Tuple<Relation> &, Tuple<Relation> &,
		      Tuple<naming_info *> &,
		      Relation &known, int effort=0);

class spmd_stmt_info;
inline String MMGenerateCode(Tuple<Relation> &tup1, Tuple<Relation> &tup2,
			     Tuple<spmd_stmt_info *> &blip,
			     Relation &known, int effort=0){
    Tuple<naming_info *> ni(0);
    foreach(s,spmd_stmt_info*,blip,ni.append((naming_info*)s));
    return MMGenerateCode(tup1,tup2,ni,known,effort);
}



String outputLBasString(const GEQ_Handle &g, Relation &bounds, Variable_ID v,
                        coef_t stride, const EQ_Handle &strideEQ, 
			Relation known);
String outputUBasString(const GEQ_Handle &g, Relation &bounds, Variable_ID v,
                        coef_t stride, const EQ_Handle &strideEQ);
void name_codegen_vars(Relation &R);

class naming_info {
public:
    naming_info(): name_debug("") {}
    virtual naming_info& operator=(const naming_info &) {assert(0); return *this; }
    virtual String name(Relation *current_map)=0;
    virtual String debug_name() const =0;
    virtual const char *debug_char_name()=0;
    virtual String declaration()=0;
    // added by D people. Lei Zhou, Apr. 25, 96
    virtual CG_outputRepr *place_holder(CG_outputBuilder *ocg,
					int indent, Relation *current_map) = 0;

    friend bool operator==(const naming_info &n1, const naming_info &n2);
protected:
  naming_info(String dn) : name_debug(dn) {}
  String name_debug; // keep this around so that it's safe to return a char*

  virtual ~naming_info();  // keep g++ happy
};

// output format: s[stmt_num](iter..)
class default_stmt_info : public naming_info {
public:
    default_stmt_info() : naming_info(""), stmt_num(-1) {}
    default_stmt_info(int arg):
	naming_info(String("s") + itoS(arg)), stmt_num(arg)
        { assert (stmt_num >= 0); }
    default_stmt_info(const default_stmt_info& n) :
	naming_info(n.name_debug), stmt_num(n.stmt_num) {}

    default_stmt_info& operator=(const default_stmt_info &n2){
	stmt_num = n2.stmt_num;
	name_debug = n2.name_debug;
	return *this;
    }

    String name(Relation *current_map) 
        {
	    print_in_code_gen_style++;
	    assert(stmt_num > 0);
		String s = String("s") + itoS(stmt_num) + "(" +
			   current_map->print_outputs_with_subs_to_string() 
			   + ");";
	    print_in_code_gen_style--;
	    return s;
	}
    String debug_name() const { assert(stmt_num > 0); return name_debug; }
    const char * debug_char_name() 
       { assert(stmt_num > 0); return (const char *) name_debug; }
    String declaration() 
        { return String("void work(int,int,...);\n"); }
    // added by D people. Lei Zhou, Apr. 25, 96
  virtual CG_outputRepr *place_holder(CG_outputBuilder* ocg,
					 int indent, Relation *current_map);


private:
  int stmt_num;
};

} // end of omega namespace

#endif
