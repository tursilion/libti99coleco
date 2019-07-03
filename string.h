// String code for the TI-99/4A by Tursi
// You can copy this file and use it at will ;)

// strlen - returns the length of a zero terminated string
int strlen(const char *s);

// returns a pointer to a static string, a number converted as unsigned
// Not thread safe, don't use from interrupt handlers.
char *uint2str(unsigned int x);

// returns a pointer to a static string, a number converted as signed
// Not thread safe, don't use from interrupt handlers.
char *int2str(int x);
