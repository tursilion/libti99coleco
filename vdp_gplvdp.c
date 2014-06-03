#include "vdp.h"
#include "grom.h"

// Copy cnt characters from a GPL copy function vectored at
// vect to VDP adr. GPL vector must be a B or BR and
// the first actual instruction must be a DEST with an
// immediate operand. 994A only (99/4 uses 6 byte chars)
// Not supported on Coleco
void gplvdp(int vect, int adr, int cnt) { }

