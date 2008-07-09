/* $Id: hull.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#ifndef Already_Included_Hull
#define Already_Included_Hull
#include <omega/farkas.h>

namespace omega {

// All of the following first call approximate on S to
// eliminate any wildcards, strides the conjuncts of S

// x in Convex Hull of S
// iff
// exist a_i, y_i s.t. 
//    x = Sum_i  a_i y_i s.t.
//    forall i, y_i in S
//    forall i, a_i >=0
//    sum_i  a_i = 1
Relation ConvexHull(NOT_CONST Relation &S);

// DecoupledConvexHull is the same as ConvexHull, 
// except that it only finds constraints that involve
// both variables x&y if there is a constraint 
// that involves both x&y in one of the conjuncts 
// of S
Relation DecoupledConvexHull(NOT_CONST Relation &S);

// x in Affine Hull of S
// iff
// exist a_i, y_i s.t. 
//    x = Sum_i  a_i y_i s.t.
//    forall i, y_i in S
//    sum_i  a_i = 1
//
// The affine hull just consists of equality constraints
// but is otherwise the tightest hull on S
Relation AffineHull(NOT_CONST Relation &S);

// x in Linear Hull of S
// iff
// exist a_i, y_i s.t. 
//    x = Sum_i  a_i y_i s.t.
//    forall i, y_i in S
//
Relation LinearHull(NOT_CONST Relation &S);

// x in Conic Hull of S
// iff
// exist a_i, y_i s.t. 
//    x = Sum_i  a_i y_i s.t.
//    forall i, y_i in S
//    forall i, a_i >=0
//
// The conic hull is the tighest cone that contains S
Relation ConicHull(NOT_CONST Relation &S);

// Given a Relation R, returns a relation deltas
// that correspond to the ConicHull of the detlas of R
Relation ConicClosure (NOT_CONST Relation &R);


Relation FastTightHull(NOT_CONST Relation &input_R,
                        NOT_CONST Relation &input_H);

Relation CheckForConvexPairs(NOT_CONST Relation &S);

Relation  Hull(NOT_CONST Relation &R, 
			bool stridesAllowed = false,
			int effort=1,
			NOT_CONST Relation &knownHull = Relation::Null()
			);

Relation Hull(Tuple<Relation> &Rs, 
		Tuple<int> &validMask, 
		int effort = 1, 
		bool stridesAllowed = false,
		NOT_CONST Relation &knownHull = Relation::Null()
		);


Relation VennDiagramForm(
                Tuple<Relation> &Rs,
                NOT_CONST Relation &Context_In);

Relation VennDiagramForm(
                NOT_CONST Relation &R_In,
                NOT_CONST Relation &Context_In = Relation::Null());


Relation CheckForConvexRepresentation(NOT_CONST Relation &R_In);

} // end of namespace omega

#endif
