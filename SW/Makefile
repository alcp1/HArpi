# My makefile

# --- Compiler and Flags ---
CC = gcc
# CFLAGS:
#   -lpaho-mqtt3c: paho mqtt library
#   -pthread: sdds support for multithreading
#   -Wall: Enable all standard warnings
#   -ljson-c: json-c library
#   -g: retain source-level debugging/symbol information
#   -I../HMSG/SW/source/: Add '../HMSG/SW/source/' directory to include search path

# REMARK: When you use -I<directory>, the C compiler searches for included header files in the specified directories in the order they appear on the command line.
#         Also, first match wins: If you have two directories, say dirA and dirB, and both contain a file named my_header.h, and your CFLAGS include -IdirA -IdirB, 
#         then if a source file does #include "my_header.h", the compiler will find and use the my_header.h from dirA first.
CFLAGS=-c                      \
         -lpaho-mqtt3c         \
         -pthread              \
         -Wall                 \
         -ljson-c              \
         -g                    \
         -Isource/             \
         -I../HMSG/SW/source/  \

LDFLAGS = -lpaho-mqtt3c -pthread -ljson-c -g

# --- Project Configuration ---
# Name of the executable
TARGET = HArpi
# Directory for the final executable
BINDIR = out
# Directory for intermediate object files
OBJDIR = objects
# Directories containing source (.c) files
SRCDIRS = source ../HMSG/SW/source

# --- Source File Discovery ---
# Find all .c files in the specified source directories.
# 'foreach' iterates through SRCDIRS.
# 'wildcard' expands to all files matching the pattern within each directory.
C_IGNORE = ../HMSG/SW/source/main.c ../HMSG/SW/source/manager.c ../HMSG/SW/source/config.c ../HMSG/SW/source/jsonhandler.c
C_SOURCES = $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.c))
C_SOURCES := $(filter-out $(C_IGNORE), $(C_SOURCES))
#To print C_SOURCES:
#$(info C_SOURCES is $(C_SOURCES))

# --- Object File Generation ---
# 'notdir' extracts just the filename from each source path (e.g., src/main.c -> main.c)
# 'patsubst' substitutes the .c extension with $(OBJDIR)/.o to create the full path for object files.
OBJECTS = $(patsubst %.c,$(OBJDIR)/%.o,$(notdir $(C_SOURCES)))
#To print OBJECTS:
#$(info OBJECTS is $(OBJECTS))

# --- Phony Targets ---
# .PHONY declares targets that are not actual files, ensuring they run even if a file with the same name exists.
.PHONY: all clean

# --- Main Target: Build the Executable ---
# The 'all' target depends on the final executable.
all: $(BINDIR)/$(TARGET)

# Rule to link object files into the executable.
# Prerequisites ($^): All object files.
# Target ($@): The final executable.
$(BINDIR)/$(TARGET): $(OBJECTS)
	@echo "Linking $(TARGET)..."
	@mkdir -p $(BINDIR) # Create the bin directory if it doesn't exist
	$(CC) $^ $(LDFLAGS) -o $@

# --- Compilation Rule: .c to .o ---
# This is a pattern rule that tells make how to compile any .c file into an .o file.
# The 'VPATH' variable (defined below) is crucial here, as it tells make to look for
# the source files (%.c) in the directories listed in SRCDIRS.
# Target ($@): The object file (e.g., obj/main.o)
# Source ($<): The corresponding source file (e.g., src/main.c or src/utils/utils.c)
$(OBJDIR)/%.o: %.c
	@echo "Compiling $< to $@"
	@mkdir -p $(OBJDIR) # Create the obj directory if it doesn't exist
	$(CC) $(CFLAGS) -c $< -o $@

# --- VPATH (Virtual Path) ---
# VPATH tells make where to search for prerequisites that are not in the current directory.
# This is essential for the pattern rule `$(OBJDIR)/%.o: %.c` to find the .c source files.
VPATH = $(SRCDIRS)

# --- Clean Target ---
# Removes all generated object files and the executable.
clean:
	@echo "Cleaning up..."
	@rm -rf $(OBJDIR) $(BINDIR)/$(TARGET)