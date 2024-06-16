// ======================================================================
//                            AurorOS Kernel
// ======================================================================

// => The initial API for console commands.
//       This API is used to execute commands and run programs using
//       console API calls.

#include <string.h>
#include "api.h"

// Function to execute a command with arguments
int execute_command(const char *command, char *args[]) {
    // Check if command is NULL or empty
    if (command == NULL || command[0] == '\0') {
        write_error("Please enter a command!\n");
        return 1;
    }

    // Execute command based on command and args
    if (strcmp(command, "help") == 0) {
    } else {
        write_error("Type a valid command!\n");
        return 1;
    }

    return 0; // Return success
}