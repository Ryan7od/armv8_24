#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#define strlen(arr) = sizeOf(arr)/sizeOf(arr[0])

//Prototype functions
extern void printEnd(FILE *ptr);



int main(int argc, char **argv) {
  //Check 1 or 2 arguments
  if (argc != 1 && argc != 2) return -1;

  //Set output method
  FILE *outPtr = stdout;
  if (argc > 1) {
    outPtr = fopen(argv[1], 'w');
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
