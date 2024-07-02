# ========================================================
#                      AurorOS Makefile
# ========================================================

# THIS MAKEFILE WORKS ON LINUX!
# Don't try to run this Makefile on Windows or other OS'es.

# Don't learn how to build the AurorOS using this file!
# Use Makefile to get the latest compilation mode!

# Define flags for GCC
CFLAGS = -Wall -Wextra -O2 -nostdinc -ffreestanding -fno-builtin -Iinclude

# Define drivers C files
DRIVERS_FILES = drivers/console/init.c drivers/vga/console.c

.PHONY: all help kernel build clean

# Define rule for normal `make`
all:
	@echo "======================================================"
	@echo "                   AurorOS Makefile"
	@echo "======================================================"
	@echo 'Write "make help" to get help.'

# Define rule for `make help`
help:
	@echo "Available commands:"
	@echo "  make help    - Show this help message"
	@echo "  make kernel  - Build the kernel binary"
	@echo "  make clean   - Clean the output directory"

# Define rules for compiling kernel
kernel:
	@echo Building kernel...
	@mkdir -p out
	gcc ${CFLAGS} kernel/main.c ${DRIVERS_FILES} -o out/kernel.bin

# Define rule for cleaning.
clean:
	@echo Cleaning output directory...
	@rm -rf out