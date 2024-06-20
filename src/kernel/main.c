// ======================================================================
//                            AurorOS Kernel
// ======================================================================

// => The AurorOS kernel.
//       This file is basically an system loading from the kernel
//       and inplementation of console commands.

// Include C basics
#include <string.h>

// Get kernel functions
#include "functions.h"

// Get console API
#include "../console/api.h"

// Reset to default color
set_text_color(0x07);

// Write AurorOS version
print_version_line();

// Say that kernel parts has been included
write_OK("Loaded: AurorOS kernel\n");

// Function executes before multitasking cycle
int system_loop() {
    return 0; // No errors
}

// Function to use multitasking in AurorOS
int multitasking_loop() {
    return 0; // No errors
}

// The main kernel loop
while (true) {
    // Execute system loop
    system_loop();

    // Execute multitasking things (like launching an asynchonic program)
    multitasking_loop();
}