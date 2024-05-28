#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#define strlen(arr) = sizeOf(arr)/sizeOf(arr[0])

//ADTs
typedef uint64_t Register;

struct PSTATE {
  bool N = false;
  bool Z = true;
  bool C = false;
  bool V = false;
};

struct GeneralRegisters {
  Register registers[32] = { 0 };
};

struct SpecialRegisters {
  Register PC = 0;
  Register Zero = 0;
  struct PSTATE pstate;
};

//Prototype functions
extern void printEnd(FILE *ptr);

//Global variables
//2MB of memory
char* memory[2097152] = { 0 };
struct GeneralRegisters gRegisters;
struct SpecialRegisters sRegisters;

int main(int argc, char **argv) {
  //Check 1 or 2 arguments
  if (argc != 1 && argc != 2) {
    printf("Wrong number of arguments");
    return -1;
  }

  //Set output method
  FILE *outPtr = stdout;
  if (argc > 1) {
    outPtr = fopen(argv[1], "w");
    if (outPtr == NULL) {
      printf("Error opening %s", argv[1]);
      return -1;
    }
  }

  //Read in file
  FILE *inPtr = fopen(argv[0], "rb");


  printEnd(outPtr);
  fcloseall();
  return EXIT_SUCCESS;
}

void printEnd(FILE *ptr) {

}
