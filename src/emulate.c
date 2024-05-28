#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#define strlen(arr) = sizeof(arr)/sizeof(arr[0])

//ADTs
typedef uint64_t Register;

struct PSTATE {
  bool N;
  bool Z;
  bool C;
  bool V;
};

struct SpecialRegisters {
  Register PC;
  Register Zero;
  struct PSTATE pstate;
};

//Prototype functions
extern void printEnd(FILE *ptr);
extern void write32(Register* reg, int32_t val);
extern int32_t read32(Register* reg);
extern void write64(Register* reg, int64_t val);
extern int64_t read64(Register* reg);

//Global variables
//2MB of memory
unsigned char memory[2097152] = { 0 };
Register gRegisters[32] = { 0 };
struct SpecialRegisters sRegisters = { 0, 0, { false, true, false, false } };
#define zeroReg &sRegisters.Zero

int main(int argc, char **argv) {
  //Check 1 or 2 arguments
  if (argc != 3 && argc != 2) {
    printf("Wrong number of arguments");
    return -1;
  }

  //Set output method
  FILE *outPtr = stdout;
  if (argc > 2) {
    outPtr = fopen(argv[2], "w");
    if (outPtr == NULL) {
      printf("Error opening %s", argv[2]);
      return -1;
    }
  }

  //Read in file
  FILE *inPtr;
  inPtr = inPtr = fopen(argv[1], "rb");
  unsigned char buffer[4] = { 0 };
  unsigned char* memPtr = memory;
  if(inPtr == NULL) return -1;
  while (fread(buffer, sizeof(buffer), 1, inPtr)) {
    for (int i = 0; i < 4; i++) {
      *memPtr++ = buffer[i];
    }
  }


  printEnd(outPtr);
  //fcloseall();
  return EXIT_SUCCESS;
}

//Prints out final states
void printEnd(FILE *ptr) {
  for (int i = 0; i < 32; i++) {
    fprintf(ptr, "X%i = %lx\n", i, gRegisters[i]);
  }
  fprintf(ptr, "PC = %lx\n", sRegisters.PC);
  fprintf(ptr, "PSTATE: ");
  if(sRegisters.pstate.N) fprintf(ptr, "N"); else fprintf(ptr, "-");
  if(sRegisters.pstate.Z) fprintf(ptr, "Z"); else fprintf(ptr, "-");
  if(sRegisters.pstate.C) fprintf(ptr, "C"); else fprintf(ptr, "-");
  if(sRegisters.pstate.V) fprintf(ptr, "V\n"); else fprintf(ptr, "-\n");
  //Output used memory
}

//Writes unless zero reg, since 32 bits, moves to the front of register by LSL
void write32(Register* reg, int32_t val) {
  if (reg == zeroReg) return;
  *reg = ((int64_t) val) << 32;
}

//Writes unless zero reg
void write64(Register* reg, int64_t val) {
  if (reg == zeroReg) return;
  *reg = val;
}

//Reads, since only first 32 bits LSR
int32_t read32(Register* reg) {
  return *reg >> 32;
}

//Reads
int64_t read64(Register* reg) {
  return *reg;
}
