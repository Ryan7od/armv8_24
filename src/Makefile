# Define the directory containing the source files
EMU_DIR := ./1_emu
ASM_DIR := ./2_asm
PI_DIR  := ./3_pi
EXT_DIR := ./4_ext

ALL_DIR := $(EMU_DIR) $(ASM_DIR) $(PI_DIR) $(EXT_DIR)

# Define the compiler and compiler flags
CC := gcc
CFLAGS := -Wall -Werror -I.

# Define the source files, with emulate.c at the end
EMU_SRCS := $(EMU_DIR)/utility.c \
        $(EMU_DIR)/branches.c \
        $(EMU_DIR)/dpimmediate.c \
        $(EMU_DIR)/dpregister.c \
        $(EMU_DIR)/loadstore.c \
        $(EMU_DIR)/emulate.c

ASM_SRCS := $(ASM_DIR)/assemble.c

EXT_SRCS := $(EXT_DIR)/morse.c

# Define the object files (replacing .c with .o)
EMU_OBJS := $(EMU_SRCS:.c=.o)
ASM_OBJS := $(ASM_SRCS:.c=.o)
EXT_OBJS := $(EXT_SRCS:.c=.o)

# Define rule to compile all
all: emulate assemble extension


# Rule to link the object files into the executable
emulate: $(EMU_OBJS)
	$(CC) $(EMU_OBJS) -o $@

assemble: $(ASM_OBJS)
	$(CC) $(ASM_OBJS) -o $@

extension: $(EXT_OBJS)
	$(CC) $(EXT_OBJS) -o $@

# Rule to compile source files into object files
$(EMU_DIR)/%.o: $(EMU_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(ASM_DIR)/%.o: $(ASM_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

        
$(EXT_DIR)/%.o: $(EXT_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Rule to clean the build directory
.PHONY: clean
clean:
	rm -f $(EMU_OBJS) emulate $(ASM_OBJS) assemble $(EXT_OBJS) extension
