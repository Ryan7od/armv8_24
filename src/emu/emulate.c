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
static FILE* getOutputPtr( int argc, char **argv );
static void readInBinFileMem( char * binaryFileName );
static void emulate( void );

static void printFinalState( int argc, char **argv );
static void printRegisters( FILE *outPtr );
static void printPC( FILE *outPtr );
static void printPSTATE( FILE *outPtr );
static void printNonZeroMem( FILE *outPtr );

//Global variables
unsigned char memory[MB2] = { 0 };
Register gRegisters[31] = { 0 };
SpecialRegisters sRegisters = { 0, 0, { false, true, false, false } };


int main(int argc, char **argv) {
  
  //Ensure 1 or 2 arguments
  if (argc != 3 && argc != 2) {
    ERROR_A("Incorrect number of arguments: %d, should be 1 or 2", (argc - 1));
  }
  
  readInBinFileMem(argv[1]);

  emulate();
  
  printFinalState(argc, argv);

  return EXIT_SUCCESS;
}



// Reads in binary file to memory
static void readInBinFileMem( char* binaryFileName ) {
  //Read in file
  FILE *inPtr;
  inPtr = fopen(binaryFileName, "rb");
  unsigned char buffer[4] = { 0 };
  unsigned char* memPtr = memory;
  if (inPtr == NULL) {
    ERROR_A("Specified binary file, %s, does not exist", binaryFileName);
  }
  while (fread(buffer, sizeof(buffer), 1, inPtr)) {
    for (int i = 0; i < 4; i++) {
      *memPtr++ = buffer[i];
    }
  }
  fclose(inPtr);
}

static void emulate( void ) {
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
}

//Prints out final state
static void printFinalState(int argc, char **argv) {
  
  FILE *outPtr = getOutputPtr(argc, argv);

  printRegisters(outPtr);

  printPC(outPtr);

  printPSTATE(outPtr);

  printNonZeroMem(outPtr);

  if (argc == 2) {
    fclose(outPtr);
  }
}

static FILE* getOutputPtr( int argc, char **argv ) {
  //Set output method MOVE TO OUTPUT
  FILE *outPtr = stdout;
  if (argc > 2) {
    outPtr = fopen(argv[2], "w");
    if (outPtr == NULL) {
      ERROR_A("Specified output method, %s, does not exist", argv[2]);
    }
  }
  return outPtr;
}

//Output 31 general registers
static void printRegisters(FILE *outPtr) {
  for (int i = 0; i < 31; i++) {
    // Formatted output using register values
    if (i < 10) {
      fprintf(outPtr, "X0%d = %016llx\n", i, (unsigned long long)read64(&gRegisters[i]));
    } else {
      fprintf(outPtr, "X%d = %016llx\n", i, (unsigned long long)read64(&gRegisters[i]));
    }
  }
}

//Output Program Counter
static void printPC(FILE *outPtr) {
  fprintf(outPtr, "PC  = %016llx\n", (unsigned long long)read64(&sRegisters.PC));
}

// Output PSTATE
static void printPSTATE(FILE *outPtr) {
  fprintf( outPtr, "PSTATE: " );

  fprintf( outPtr, sRegisters.pstate.N ? "N" : "-" );
  fprintf( outPtr, sRegisters.pstate.Z ? "Z" : "-" );
  fprintf( outPtr, sRegisters.pstate.C ? "C" : "-" );
  fprintf( outPtr, sRegisters.pstate.V ? "V" : "-" );
  
  fprintf( outPtr, "\n" );
}

// Outputs non 0 memory
static void printNonZeroMem(FILE *outPtr) {
  for (int i = 0; i < MB2; i += 4) {
    long unsigned int ith_word_memory = fetch32(i);
    
    // Only output if not 0
    if (ith_word_memory) {
      fprintf(outPtr, "0x%08lx: %08lx\n", (long unsigned int)i, ith_word_memory);
    }
  }
}
