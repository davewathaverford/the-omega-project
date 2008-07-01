/* $Id: wak_codegen.h,v 1.1.1.1 2000/06/29 19:24:41 dwonnaco Exp $ */
#ifndef Already_Included_Wak_CodeGen
#define Already_Included_Wak_CodeGen

extern String WakGenerateCode(String init);

extern Relation wak_project_onto_levels(Relation R, int last_level, bool wildcards);

extern String outputEasyBound(const GEQ_Handle &g, Variable_ID v, 
			      int lower_bound);

extern int is_loop_dist[max_stmts][maxLevels];

#endif
