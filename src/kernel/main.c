// ======================================================================
//                            AurorOS Kernel
// ======================================================================

// => The AurorOS kernel.
//       This file is basically an system loading from the kernel
//       and inplementation of console commands.

// Get console API
#include "../console/api.h"

// Get AurorOS filesystem API
#include "../drivers/fs/afs.h"

// Get NTFS filesystem API
#include "../drivers/fs/ntfs.h"

// Reset to default color
set_text_color(0x07);

// Say that kernel parts has been included
write_OK("Console API");
write_OK("AFS driver");
write_OK("NTFS driver");