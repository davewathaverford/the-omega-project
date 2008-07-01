	{ if (not_first_time) printf "|"; }
	{ printf("(%s)", $0); }
	{ not_first_time = 1; }
END	{ printf("\n"); }
