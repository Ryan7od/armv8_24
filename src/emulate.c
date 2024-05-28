#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#define strlen(arr) = sizeof(arr)/sizeof(arr[0])
#define zeroReg &sRegisters.Zero
#define MB2 2097152

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
extern void write32(Register* reg, uint32_t val);
extern uint32_t read32(Register* reg);
extern void write64(Register* reg, uint64_t val);
extern uint64_t read64(Register* reg);
extern uint32_t fetch32(int index);
extern void dataProcessingImmHandler(uint32_t instruction);
extern void dataProcessingRegHandler(uint32_t instruction);
extern void loadStoreHandler(uint32_t instruction);
extern void branchHandler(uint32_t instruction);

//Global variables
//2MB of memory
unsigned char memory[MB2] = { 0 };
Register gRegisters[32] = { 0 };
struct SpecialRegisters sRegisters = { 0, 0, { false, true, false, false } };

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

  //Run through each step
  uint32_t instruction = fetch32(sRegisters.PC);
  //Terminate at 8a000000
  while(instruction != 0x8a000000) {
    //Switch case to send to different operation handlers
    switch (instruction & 0b00011110000000000000000000000000) {
      case (0b00010000000000000000000000000000):
      case (0b00010010000000000000000000000000): {
        dataProcessingImmHandler(instruction);
      }
      case (0b00001010000000000000000000000000):
      case (0b00011010000000000000000000000000): {
        dataProcessingRegHandler(instruction);
      }
      case (0b00011000000000000000000000000000):
      case (0b00001100000000000000000000000000):
      case (0b00001000000000000000000000000000):
      case (0b00011100000000000000000000000000): {
        loadStoreHandler(instruction);
      }
      case (0b00010100000000000000000000000000):
      case (0b00010110000000000000000000000000): {
        branchHandler(instruction);
        break;
      }
      default: {

      }
    }

    write64(&sRegisters.PC, sRegisters.PC + 4);
    instruction = fetch32(sRegisters.PC);
  }




  //printEnd(outPtr);
  //fcloseall();
  return EXIT_SUCCESS;
}

//Prints out final states
void printEnd(FILE *ptr) {
  for (int i = 0; i < 32; i++) {
    if (i < 10) fprintf(ptr, "X%i  = %016lx\n", i, gRegisters[i]);
    else fprintf(ptr, "X%i = %016lx\n", i, gRegisters[i]);
  }
  fprintf(ptr, "PC  = %016lx\n", sRegisters.PC);
  fprintf(ptr, "PSTATE: ");
  if(sRegisters.pstate.N) fprintf(ptr, "N"); else fprintf(ptr, "-");
  if(sRegisters.pstate.Z) fprintf(ptr, "Z"); else fprintf(ptr, "-");
  if(sRegisters.pstate.C) fprintf(ptr, "C"); else fprintf(ptr, "-");
  if(sRegisters.pstate.V) fprintf(ptr, "V\n"); else fprintf(ptr, "-\n");
  //Output used memory
}

//Writes unless zero reg, since 32 bits, moves to the front of register by LSL
void write32(Register* reg, uint32_t val) {
  if (reg == zeroReg) return;
  *reg = ((int64_t) val) << 32;
}

//Writes unless zero reg
void write64(Register* reg, uint64_t val) {
  if (reg == zeroReg) return;
  *reg = val;
}

//Reads, since only first 32 bits LSR
uint32_t read32(Register* reg) {
  return *reg >> 32;
}

//Reads
uint64_t read64(Register* reg) {
  return *reg;
}

uint32_t fetch32(int index) {
  if (index + 3 >= 2097152) return -1;
  return memory[index] + memory[index+1] << 4 + memory[index+2] << 8 + memory[index+3] << 12;
}
