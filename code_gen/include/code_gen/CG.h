
#if !defined(Already_included_cg)
#define Already_included_cg

// Code generation "CG" structures; class declarations

#include <basic/bool.h>
#include <basic/Tuple.h>
#include <omega/Relation.h>
#include <code_gen/elim.h>

namespace omega {

typedef Tuple<int> IntTuple;
typedef Tuple<Relation> SetTuple;
typedef Tuple<SetTuple> SetTupleTuple;
typedef Tuple<Relation> RelTuple;

class CG_result {
public:
    IntTuple  isActive;
    virtual CG_result * new_copy() = 0;
    virtual String print(int indent) = 0;
    virtual String printStructure(int indent) = 0;
    virtual int depth() = 0;
    // added by D people
    virtual CG_outputRepr* printRepr(CG_outputBuilder* ocg, int indent) = 0;

    int level;
    Relation restrictions;
    Relation known;
    elimination_info eliminated;
    virtual CG_result * recompute(const Relation k, const Relation r,
				  elimination_info e) = 0;
    virtual bool isNull() { return 0;};
    virtual CG_result * liftOverhead(int depth) = 0;
    virtual Relation findOverhead(int liftTo) = 0;
    virtual CG_result *force_finite_bounds() = 0;
    virtual bool verify_overhead_removal(int depth);

  virtual ~CG_result();  // keep g++ happy
};


class CG_null : public CG_result {
public:
    virtual CG_result * new_copy() {
	return new CG_null;
    }
    virtual String print(int) {
	return (String)"";
    }
    virtual String printStructure(int) {
	return (String)"";
    }
    // added by D people
    virtual CG_outputRepr* printRepr(CG_outputBuilder*, int) 
      { return (CG_outputRepr *)0; };

    virtual int depth() { return 0; }
    virtual bool isNull() { return 1;};
    virtual CG_result * recompute(const Relation, const Relation,
				  elimination_info ) {
	return this;
    }
    virtual CG_result * liftOverhead(int) {
	return this;
    }
    virtual Relation findOverhead(int) {
	return Relation::True(1);
    }
    virtual CG_result *force_finite_bounds() { return this; };
    virtual bool verify_overhead_removal(int depth);
};



class CG_split : public CG_result {
public:
    CG_split(IntTuple &active, int lvl, Relation cond,
	     CG_result *T, CG_result *F);
    ~CG_split() { delete trueClause; delete falseClause; }
    CG_result *new_copy();
    virtual int depth() 
      { return omega::max(trueClause->depth(),falseClause->depth()); }
    virtual String printStructure(int indent);
    virtual String print(int indent);
    // added by D people
    virtual CG_outputRepr* printRepr(CG_outputBuilder* ocg, int indent);

    virtual CG_result * liftOverhead(int depth);
    virtual CG_result * force_finite_bounds();
    virtual bool verify_overhead_removal(int depth);
    virtual Relation findOverhead(int liftTo);
    virtual CG_result * recompute(const Relation k, const Relation r,
				  elimination_info e);
    CG_result *trueClause,*falseClause;
    Relation condition;
};




class CG_leaf : public CG_result {
public:
    CG_leaf(IntTuple &active) {
	isActive = active;
	guard.reallocate(isActive.size());
    }
    CG_result *new_copy() {
	return new CG_leaf(isActive);
    }
    virtual CG_result * liftOverhead(int) {
	return this;
    }
    virtual CG_result * force_finite_bounds() {
	return this;
    }
    virtual bool verify_overhead_removal(int depth);
    virtual Relation findOverhead(int liftTo);
    ~CG_leaf() { };
    virtual String print(int indent);
    virtual String printStructure(int indent);
    // added by D people
    virtual CG_outputRepr* printRepr(CG_outputBuilder* ocg, int indent);
    virtual int depth() { return 0; }
    virtual CG_result * recompute(const Relation k, const Relation r,
				  elimination_info e);
    SetTuple guard;
};

class CG_loop : public CG_result {
public:
    CG_loop(IntTuple &active, int lvl, CG_result *b) {
	isActive = active;
	level = lvl;
	body = b;
    };
    ~CG_loop() { delete body; }
    CG_result *new_copy() {
	return new CG_loop(isActive,level,body->new_copy());
    }
    virtual Relation findOverhead(int liftTo);
    virtual CG_result * force_finite_bounds();
    virtual bool verify_overhead_removal(int depth);
    virtual CG_result * liftOverhead(int depth);
    virtual String print(int indent);
    virtual String printStructure(int indent);
    // added by D people
    virtual CG_outputRepr* printRepr(CG_outputBuilder* ocg, int indent);
    virtual int depth() { return ((int)needLoop)+body->depth(); }
    virtual CG_result * recompute(const Relation k, const Relation r,
				  elimination_info e);
    Relation bounds;
    Relation guard;
    bool needLoop;
    CG_result *body;
};

} // end omega namespace
#endif
