#include "vdp.h"
#include "halt.h"

// tested

void halt() {
	// no 'quit' on Coleco - but maybe someday we'll add it.
	VDP_INT_ENABLE;
	for (;;) { }
}
