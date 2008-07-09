#if defined STUDY_EVACUATIONS


namespace omega {

// study the evacuation from one side of C to the other for UFS's of
// arity up to max_arity
extern void study_evacuation(Conjunct *C, which_way dir, int max_arity);

// study the evacuation from the joined C2's output and C1's input to
// either of the other possible tuples
extern void study_evacuation(Conjunct *C1, Conjunct *C2, int max_arity);


} // end of namespace omega

#endif
