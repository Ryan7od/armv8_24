#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#define strlen(arr) = sizeof(arr)/sizeof(arr[0])

//ADTs
typedef int64_t Register;

struct PSTATE {
  bool N;
  bool Z;
  bool C;
  bool V;
};

struct GeneralRegisters {
  Register registers[32];
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
struct GeneralRegisters gRegisters = { { 0 } };
struct SpecialRegisters sRegisters= { 0, 0, { false, true, false, false } };

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
  FILE *inPtr = fopen(argv[1], "rb");
  unsigned char buffer[4] = { 0 };
  unsigned char* memPtr = memory;
  fread(buffer, sizeof(buffer), 1, inPtr);
  while (buffer[0] != -1) {
    *memPtr++ = buffer[0];
    *memPtr++ = buffer[1];
    *memPtr++ = buffer[2];
    *memPtr++ = buffer[3];
    fread(buffer, sizeof(buffer), 1, inPtr);
  }

  for(int i = 0; i < 8; i++) {
    printf("%u ", memory[i]);
    if ((i+1)%4 == 0) printf("\n");
  }


  printEnd(outPtr);
  fcloseall();
  return EXIT_SUCCESS;
}

void printEnd(FILE *ptr) {

}

void write32(Register* reg, int32_t val) {
  *reg = ((int64_t) val) << 32;
}

void write64(Register* reg, int64_t val) {
  *reg = val;
}

int32_t read32(Register* reg) {
  return *reg >> 32;
}

int64_t read64(Register* reg) {
  return *reg;
}
