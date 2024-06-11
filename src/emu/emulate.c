#ifndef emulate
#define emulate

#include "branches.h"
#include "dpimmediate.h"
#include "dpregister.h"
#include "memReg.h"
#include "utility.h"
#include "loadstore.h"

#endif

//Prototype functions
extern void printEnd(FILE *ptr);


extern void branchHandler(uint32_t instruction);

int main(int argc, char **argv) {
  //Ensure 1 or 2 arguments
  if (argc != 3 && argc != 2) {
    fprintf(stderr, "Error: wrong number of arguments");
    return -1;
  }

  //Set output method
  FILE *outPtr = stdout;
  if (argc > 2) {
    outPtr = fopen(argv[2], "w");
    if (outPtr == NULL) {
      fprintf(stderr, "Error: opening %s", argv[2]);
      return -2;
    }
  }

  //Read in file
  FILE *inPtr;
  inPtr = fopen(argv[1], "rb");
  unsigned char buffer[4] = { 0 };
  unsigned char* memPtr = memory;
  if (inPtr == NULL) {
    fprintf(stderr, "Error: File is null & doesn't exist");
    return -3;
  }
  while (fread(buffer, sizeof(buffer), 1, inPtr)) {
    for (int i = 0; i < 4; i++) {
      *memPtr++ = buffer[i];
    }
  }
  
  //Run through each step
  uint32_t instruction = fetch32(sRegisters.PC);

  //Terminate at halt (8a000000)
  while(instruction != 0x8a000000) {
    //Define skip for if branch set
    bool skip = false;
    //Defined mask & isolate bits 28-5 to get op0
    uint8_t op0 = mask32_AtoB_shifted(instruction, 28, 25);
    //Switch case to send to different operation handlers
    switch (op0) {
      //100x Data Processing (Immediate)
      case (0b1000):
      case (0b1001):
        dataProcessingImmHandler(instruction);
        break;
      //x101 Data Processing (Register)
      case (0b0101):
      case (0b1101):
        dataProcessingRegHandler(instruction);
        break;
      //x1x0 Loads and Stores
      case (0b0100):
      case (0b0110):
      case (0b1100):
      case (0b1110):
        loadStoreHandler(instruction);
        break;
      //101x Branches
      case (0b1010):
      case (0b1011):
        branchHandler(instruction);
        skip = true;
        break;
      default:
        fprintf(stderr, "Error: Non implemented instruction: %x (main)\n", op0);
        return -4;
    }

    if (!skip) {
      write64(&sRegisters.PC, read64(&sRegisters.PC) + 4);
    }
    instruction = fetch32(sRegisters.PC);
  }

  printEnd(outPtr);
  //fcloseall();
  return EXIT_SUCCESS;
}



//Branches are either register, immediate or immediate conditional, based on the first 3 bits
void branchHandler(uint32_t instruction) {
  const uint32_t diff = mask32_AtoB_shifted(instruction, 31, 29);

  //Register - 110
  if (diff == 0b110) {
    //Not zero register
    uint32_t location = mask32_AtoB_shifted(instruction, 9, 5);
    if (location != 0b11111) {
      write64(&sRegisters.PC, read64(&gRegisters[location]));
    } else {
      fprintf(stderr, "Error: Register jump to ZR; not implemented");
    }
    return;
  }

  //Unconditional - 000
  if (diff == 0b000) {
    //Find base shift
    uint32_t shift = mask32_AtoB_shifted(instruction, 25, 0) << 2;
    //Extend sign if MSB is 1
    shift = sExtend32(shift, 27);
    write64(&sRegisters.PC, read64(&sRegisters.PC) + shift);
    return;
  }

  //Conditional - 010
  if (diff == 0b010) {
    //Tackle condition
    bool condition = false;
    uint16_t encode = mask32_AtoB_shifted(instruction, 3, 0);
    switch (encode) {
      case (0b0000): // EQ
        condition = (sRegisters.pstate.Z == 1);
        break;
      case (0b0001): // NE
        condition = sRegisters.pstate.Z == 0;
        break;
      case (0b1010): // GE
        condition = sRegisters.pstate.N == sRegisters.pstate.V;
        break;
      case (0b1011): // LT
        condition = sRegisters.pstate.N != sRegisters.pstate.V;
        break;
      case (0b1100): // GT
        condition = sRegisters.pstate.Z == 0 && sRegisters.pstate.N == sRegisters.pstate.V;
        break;
      case (0b1101):
        condition = !(sRegisters.pstate.Z == 0 && sRegisters.pstate.N == sRegisters.pstate.V);
        break;
      case (0b01110): //
        condition = true;
        break;
      default:
        fprintf(stderr, "Error: Not implemented branch conditional hex code: %x", encode);
        break;
    }

    if (!condition) { // Move onto next PC address if condition not met
      write64(&sRegisters.PC, read64(&sRegisters.PC) + 4);
      return;
    }

    uint32_t simm19 = mask32_AtoB_shifted(instruction, 23 ,5);
    //Find base shift
    uint64_t shift = simm19 << 2;
    //Extend sign if MSB is 1
    shift = sExtend64(shift, 20);
    uint64_t newPC = read64(&sRegisters.PC) + shift;
    write64(&sRegisters.PC, newPC);
    return;
  }
}

//Prints out final states
void printEnd(FILE *ptr) {
  for (int i = 0; i < 31; i++) {
    if (i < 10) fprintf(ptr, "X0%i = %016llx\n", i, read64(&gRegisters[i]));
    else fprintf(ptr, "X%i = %016llx\n", i, read64(&gRegisters[i]));
  }
  fprintf(ptr, "PC  = %016llx\n", read64(&sRegisters.PC));
  fprintf(ptr, "PSTATE: ");
  if(sRegisters.pstate.N) fprintf(ptr, "N"); else fprintf(ptr, "-");
  if(sRegisters.pstate.Z) fprintf(ptr, "Z"); else fprintf(ptr, "-");
  if(sRegisters.pstate.C) fprintf(ptr, "C"); else fprintf(ptr, "-");
  if(sRegisters.pstate.V) fprintf(ptr, "V\n"); else fprintf(ptr, "-\n");
  for (int i = 0; i < MB2; i += 4) {
    uint32_t word = fetch32(i);
    if (word) {
      fprintf(ptr, "0x%08lx: %08lx\n", i, word);
    }
  }
}