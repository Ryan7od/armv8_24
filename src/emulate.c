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
extern uint32_t twos(uint32_t num);

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
      //100x Data Processing (Immediate)
      case (0b00010000000000000000000000000000):
      case (0b00010010000000000000000000000000):
        dataProcessingImmHandler(instruction);
        break;
      //x101 Data Processing (Register)
      case (0b00001010000000000000000000000000):
      case (0b00011010000000000000000000000000):
        dataProcessingRegHandler(instruction);
        break;
      //x1x0 Loads and Stores
      case (0b00011000000000000000000000000000):
      case (0b00001100000000000000000000000000):
      case (0b00001000000000000000000000000000):
      case (0b00011100000000000000000000000000):
        loadStoreHandler(instruction);
        break;
      //101x Branches
      case (0b00010100000000000000000000000000):
      case (0b00010110000000000000000000000000):
        branchHandler(instruction);
        break;
      default:
        break;
    }

    write64(&sRegisters.PC, read64(&sRegisters.PC) + 4);
    instruction = fetch32(sRegisters.PC);
  }




  printEnd(outPtr);
  //fcloseall();
  return EXIT_SUCCESS;
}

void dataProcessingImmHandler(uint32_t instruction) {

}

void dataProcessingRegHandler(uint32_t instruction) {

}

void loadStoreHandler(uint32_t instruction) {

}

//Branches are either register, immediate or immediate conditional, based on the first 3 bits
void branchHandler(uint32_t instruction) {
  //Register - 110
  if ((instruction & 0b11100000000000000000000000000000) == 0b11000000000000000000000000000000) {
    //Not zero register
    if ((instruction & 0b00000000000000000000001111110000) != 0b00000000000000000000001111110000) {
      uint32_t location = (instruction & 0b00000000000000000000001111110000) >> 4;
      write64(&sRegisters.PC, read64(&gRegisters[location]));
    }
    return;
  }

  //Unconditional - 000
  if ((instruction & 0b11100000000000000000000000000000) == 0) {
    //Find base shift
    uint32_t shift = (instruction & 0b00000011111111111111111111111111) << 2;
    //Extend sign if MSB is 1
    if (shift & 0b00001000000000000000000000000000) {
      shift = instruction | 0b11111100000000000000000000000000;
      write64(&sRegisters.PC, read64(&sRegisters.PC) - twos(shift));
    } else {
      write64(&sRegisters.PC, read64(&sRegisters.PC) + shift);
    }
    return;
  }

  //Conditional - 010
  if ((instruction & 0b11100000000000000000000000000000) == 0b01000000000000000000000000000000) {
    //Tackle condition
    bool condition = false;
    switch (instruction & 0b00000000000000000000000000001111) {
      case (0b00000):
        condition = sRegisters.pstate.Z == 1;
        break;
      case (0b00001):
        condition = sRegisters.pstate.Z == 0;
        break;
      case (0b01010):
        condition = sRegisters.pstate.N == sRegisters.pstate.V;
        break;
      case (0b01011):
        condition = sRegisters.pstate.N != sRegisters.pstate.V;
        break;
      case (0b01100):
        condition = sRegisters.pstate.Z == 0 && sRegisters.pstate.N == sRegisters.pstate.V;
        break;
      case (0b01101):
        condition = !(sRegisters.pstate.Z == 0 && sRegisters.pstate.N == sRegisters.pstate.V);
        break;
      case (0b01110):
        condition = true;
        break;
      default:
        break;
    }

    if (!condition) return;

    //Find base shift
    uint32_t shift = (instruction & 0b00000011111111111111111111100000) >> 3;
    //Extend sign if MSB is 1
    if (shift & 0b00000000000100000000000000000000) {
      shift = instruction | 0b11111111111000000000000000000000;
      write64(&sRegisters.PC, read64(&sRegisters.PC) - twos(shift));
    } else {
      write64(&sRegisters.PC, read64(&sRegisters.PC) + shift);
    }

    return;
  }
}

//Prints out final states
void printEnd(FILE *ptr) {
  for (int i = 0; i < 32; i++) {
    if (i < 10) fprintf(ptr, "X%i  = %016lx\n", i, read64(&gRegisters[i]));
    else fprintf(ptr, "X%i = %016lx\n", i, read64(&gRegisters[i]));
  }
  fprintf(ptr, "PC  = %016lx\n", read64(&sRegisters.PC));
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

uint32_t twos(uint32_t num) {
  return (~num) + 1;
}
