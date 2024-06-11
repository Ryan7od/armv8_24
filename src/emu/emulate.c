#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "utility.h"
#include "branches.h"
#include "dpimmediate.h"
#include "dpregister.h"
#include "loadstore.h"

//Prototype functions

static void outputFinalState( FILE *outPtr );
static void outputRegisters( FILE *outPtr );
static void outputPC( FILE *outPtr );
static void outputPState( FILE *outPtr );
static void outputMemory( FILE *outPtr );

unsigned char memory[MB2] = { 0 };
Register gRegisters[31] = { 0 };
SpecialRegisters sRegisters = { 0, 0, { false, true, false, false } };


int main(int argc, char **argv) {
  //Ensure 1 or 2 arguments
  if (argc != 3 && argc != 2) {
    ERROR_A("Incorrect number of arguments: %d", (argc - 1));
  }

  //Set output method
  FILE *outPtr = stdout;
  if (argc > 2) {
    outPtr = fopen(argv[2], "w");
    if (outPtr == NULL) {
      ERROR_A("Specified output method, %s, does not exist", argv[2]);
    }
  }

  //Read in file
  FILE *inPtr;
  inPtr = fopen(argv[1], "rb");
  unsigned char buffer[4] = { 0 };
  unsigned char* memPtr = memory;
  if (inPtr == NULL) {
    ERROR_A("Specified binary file, %s, does not exist", argv[1]);
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
        ERROR_A("Non-implemented instruction: %x", instruction);
    }

    if (!skip) {
      write64(&sRegisters.PC, read64(&sRegisters.PC) + 4);
    }
    instruction = fetch32(sRegisters.PC);
  }

  outputFinalState(outPtr);
  fcloseall();
  return EXIT_SUCCESS;
}



//Prints out final state
static void outputFinalState(FILE *outPtr) {
  
  outputRegisters(outPtr);

  outputPC(outPtr);

  outputPState(outPtr);

  outputMemory(outPtr);

}

//Output 31 general registers
static void outputRegisters(FILE *outPtr) {
  for (int i = 0; i < 31; i++) {
    if (i < 10) fprintf(outPtr, "X0%i = %016llx\n", i, read64(&gRegisters[i]));
    else fprintf(outPtr, "X%i = %016llx\n", i, read64(&gRegisters[i]));
  }
}

//Output Program Counter
static void outputPC(FILE *outPtr) {
  fprintf(outPtr, "PC  = %016llx\n", read64(&sRegisters.PC));
}

// Output PSTATE
static void outputPState(FILE *outPtr) {
  fprintf(outPtr, "PSTATE: ");
  fprintf(outPtr, (sRegisters.pstate.N ? "N" : "-"));
  if(sRegisters.pstate.Z) fprintf(outPtr, "Z"); else fprintf(outPtr, "-");
  if(sRegisters.pstate.C) fprintf(outPtr, "C"); else fprintf(outPtr, "-");
  if(sRegisters.pstate.V) fprintf(outPtr, "V\n"); else fprintf(outPtr, "-\n");
}

//Outputs memory
static void outputMemory(FILE *outPtr) {
  for (int i = 0; i < MB2; i += 4) {
    uint32_t word = fetch32(i);
    if (word) {
      fprintf(outPtr, "0x%08lx: %08lx\n", i, word);
    }
  }
}


