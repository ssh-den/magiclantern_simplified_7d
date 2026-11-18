#ifndef PLATFORM_7D_206_CONSTS_H
#define PLATFORM_7D_206_CONSTS_H
#include "features.h"
/* Firmware-specific overrides on top of the 7D.203 hardware layout. */
#include "../7D.203/consts.h"

/* Use boot-d45-ch's existing AllocateMemory branch.  This instruction is
 * MOV r1,#0xD00000 before heap init; the replacement ends the heap at ML. */
#undef RSCMGR_MEMORY_PATCH_END
#define HIJACK_CACHE_HACK_BSS_END_ADDR 0xFF011F2C
#define HIJACK_CACHE_HACK_BSS_END_INSTR 0xE3A018C6

#undef GMT_FUNCTABLE
#define GMT_FUNCTABLE 0xFF511914

/* FF05A6DC is not a flush-rate patch in 2.0.6; no replacement is mapped.
 * Master-side cache hacks are not ported. */
#undef CACHE_HACK_FLUSH_RATE_SLAVE
#undef CACHE_HACK_FLUSH_RATE_MASTER
#undef CACHE_HACK_GOP_SIZE_MASTER
#endif
