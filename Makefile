# Compiler
CC = gcc

# Compiler flags
CFLAGS = -std=c2x -g\
	-D_POSIX_SOURCE -D_DEFAULT_SOURCE\
	-Wall -Werror -pedantic\
    -I./src -I./src/asm -I./src/emu

# Source directories
SRC_DIR = src
ASM_DIR = $(SRC_DIR)/asm
EMU_DIR = $(SRC_DIR)/emu

# Object files for assemble
ASSEMBLE_OBJ = $(ASM_DIR)/assemble.o

# Object files for emulate
EMU_OBJ = $(EMU_DIR)/emulate.o \
          $(EMU_DIR)/branches.o \
          $(EMU_DIR)/dpimmediate.o \
          $(EMU_DIR)/dpregister.o \
          $(EMU_DIR)/loadstore.o \
          $(EMU_DIR)/utility.o

# Targets
all: assemble emulate

assemble: $(ASSEMBLE_OBJ) $(EMU_OBJ)
	$(CC) $(CFLAGS) -o $@ $(ASSEMBLE_OBJ) $(EMU_OBJ)

emulate: $(EMU_OBJ)
	$(CC) $(CFLAGS) -o $@ $(EMU_OBJ)

# Pattern rule to compile .c files to .o files
$(ASSEMBLE_DIR)/%.o: $(ASSEMBLE_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(EMU_DIR)/%.o: $(EMU_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean rule to remove generated files
clean:
	rm -f $(ASSEMBLE_DIR)/*.o $(EMU_DIR)/*.o assemble emulate

.PHONY: all clean