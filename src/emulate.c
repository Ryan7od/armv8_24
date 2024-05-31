#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#define strlen(arr) = sizeof(arr)/sizeof(arr[0])
#define zeroReg &sRegisters.Zero
#define MB2 2097152 // 2MB of memory (2*2^20)

//ADTs
typedef uint64_t Register;

typedef struct {
  bool N;
  bool Z;
  bool C;
  bool V;
} PSTATE;

typedef struct  {
  Register PC;
  Register Zero;
  PSTATE pstate;
} SpecialRegisters;

//Prototype functions
extern void printEnd(FILE *ptr);
extern void write32(Register* reg, uint32_t val);
extern uint32_t read32(Register* reg);
extern void write64(Register* reg, uint64_t val);
extern uint64_t read64(Register* reg);
extern uint32_t fetch32(int index);
extern void dataProcessingImmHandler(uint32_t instruction);
extern void dataProcessingImmArithHandler(uint32_t instruction);
extern void dataProcessingRegHandler(uint32_t instruction);
extern void loadStoreHandler(uint32_t instruction);
extern void branchHandler(uint32_t instruction);
extern uint32_t twos(uint32_t num);
extern uint32_t mask32_AtoB_shifted(uint32_t instruction, uint8_t a, uint8_t b);
extern void setFlags(uint64_t result, uint64_t a, uint64_t b, bool addition, bool bit64);

//Global variables
//2MB of memory
unsigned char memory[MB2] = { 0 };
Register gRegisters[31] = { 0 };
SpecialRegisters sRegisters = { 0, 0, { false, true, false, false } };

int main(int argc, char **argv) {
  //Ensure 1 or 2 arguments
  if (argc != 3 && argc != 2) {
    printf("Error: wrong number of arguments");
    return EXIT_FAILURE;
  }

  //Set output method
  FILE *outPtr = stdout;
  if (argc > 2) {
    outPtr = fopen(argv[2], "w");
    if (outPtr == NULL) {
      printf("Error: opening %s", argv[2]);
      return EXIT_FAILURE;
    }
  }

  //Read in file
  FILE *inPtr;
  inPtr = inPtr = fopen(argv[1], "rb");
  unsigned char buffer[4] = { 0 };
  unsigned char* memPtr = memory;
  if (inPtr == NULL) {
      return EXIT_FAILURE;
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
    //Defined mask & isolate bits 28-5 to get op0
    uint32_t mask = 0xF << 25;
    uint8_t op0 = (instruction & mask) >> 25;
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
        break;
      default:
        printf("Error: Non implemented instruction");
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
  // Get all elements of instruction for data processing immediate
  uint8_t rd = mask32_AtoB_shifted(instruction, 4, 0);
  // Leaving operand masking to cases
  uint8_t opi = mask32_AtoB_shifted(instruction, 25, 23);
  uint8_t opc = mask32_AtoB_shifted(instruction, 30, 29);
  uint8_t sf = mask32_AtoB_shifted(instruction, 31, 31);
  switch (opi) {
    case 0b010:
      dataProcessingImmArithHandler(instruction);
      break;
    case 0b101:
      // TODO: Wide move
      break;
    default:
      // TODO: Error
      break;
  }

  // Arithmetic

  // Wide move
}

void dataProcessingImmArithHandler(uint32_t instruction) {
  uint8_t rd = mask32_AtoB_shifted(instruction, 4, 0);
  uint8_t opc = mask32_AtoB_shifted(instruction, 30, 29);
  uint8_t sf = mask32_AtoB_shifted(instruction, 31, 31);
  uint8_t sh = mask32_AtoB_shifted(instruction, 22, 22);
  uint32_t imm12 = mask32_AtoB_shifted(instruction, 21, 10);
  uint8_t rn = mask32_AtoB_shifted(instruction, 9, 5);

  //Do shift if necessary
  if (sh) {
    imm12 = imm12 << 12;
  }

  //Fetch value in Rn and sign it
  uint64_t operand;
  if (sf) {
    operand = read64(&gRegisters[rn]);
  } else {
    operand = read32(&gRegisters[rn]);
  }


  uint64_t result;
  if (opc & 0b10) { //Sub
    result = operand - imm12;
  } else { //Add
    result = operand + imm12;
  }

  if (opc & 0b01) { //Set flags
    setFlags(result, operand, imm12, !(opc & 0b10), sf);
  }


  //Find output register
  Register* output;
  if((opc & 1) && rd == 0b11111) {
    output = &sRegisters.Zero;
  } else {
    output = &gRegisters[rd];
  }

  //Set value based on sf
  if (sf) {
    write64(output, result);
  } else {
    write32(output, result);
  }
}

void dataProcessingRegHandler(uint32_t instruction) {

}

void loadStoreHandler(uint32_t instruction) {

}

//Branches are either register, immediate or immediate conditional, based on the first 3 bits
void branchHandler(uint32_t instruction) {
  uint32_t diff = mask32_AtoB_shifted(instruction, 31, 29);

  //Register - 110
  if (diff == 0b110) {
    //Not zero register
    uint32_t location = mask32_AtoB_shifted(instruction, 9, 5);
    if (location != 0b11111) {
      write64(&sRegisters.PC, read64(&gRegisters[location]));
    }
    return;
  }

  //Unconditional - 000
  if (diff == 0b000) {
    //Find base shift
    uint32_t shift = mask32_AtoB_shifted(instruction, 25, 0) << 2;
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
  if (diff == 0b010) {
    //Tackle condition
    bool condition = false;
    switch (mask32_AtoB_shifted(instruction, 3, 0)) {
      case (0b0000):
        condition = sRegisters.pstate.Z == 1;
        break;
      case (0b0001):
        condition = sRegisters.pstate.Z == 0;
        break;
      case (0b1010):
        condition = sRegisters.pstate.N == sRegisters.pstate.V;
        break;
      case (0b1011):
        condition = sRegisters.pstate.N != sRegisters.pstate.V;
        break;
      case (0b1100):
        condition = sRegisters.pstate.Z == 0 && sRegisters.pstate.N == sRegisters.pstate.V;
        break;
      case (0b1101):
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
    uint32_t shift = mask32_AtoB_shifted(instruction, 23 ,5) << 2;
    //Extend sign if MSB is 1
    if (shift & 0b100000000000000000000) {
      shift = shift | 0b11111111111000000000000000000000;
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

// Returns the instruction masked from bits a to b inclusive, ensure a >= b, returns 0 if error
uint32_t mask32_AtoB_shifted(uint32_t instruction, uint8_t a, uint8_t b) {
  // Check for invalid bit positions
  if (a < b || a > 31 || b > 31) {
    printf("Error: Invalid a: %u or b: %u", a, b);
    return 0;
  }
  // Create mask from a and b via 1 shifted by difference of a and b + 1 minus 1 then shifted by a
  uint32_t mask = ((1U << (a - b + 1)) - 1) << b;
  // Return instruction masked
  return (instruction & mask) >> b;
}

void setFlags(uint64_t result, uint64_t a, uint64_t b, bool addition, bool bit64) {
  //N is set to the sign bit of the result
  if (bit64) {
    sRegisters.pstate.N = result & 0x8000000000000000;
  } else {
    sRegisters.pstate.N = result & 0x80000000;
  }

  //Z is set only when the result is all zero
  sRegisters.pstate.Z = !result;

  //V is set when there is signed overflow/underflow
  int64_t aSigned = a;
  int64_t bSigned = b;
  int64_t max;
  int64_t min;
  if (bit64) {
    max = INT64_MAX;
    min = INT64_MIN;
  } else {
    max = INT32_MAX;
    min = INT32_MIN;
  }
  if (bSigned >= 0) { //Check if a + b > max
    sRegisters.pstate.V = aSigned > max - bSigned;
  } else { //Check if a + b < min
    sRegisters.pstate.V = aSigned < min - bSigned;
  }

  uint64_t umax;
  uint64_t umin;
  if (bit64) {
    umax = UINT64_MAX;
  } else {
    umax = UINT32_MAX;
  }
  //C in addition is set when an unsigned carry produced
  if (addition) {
    sRegisters.pstate.C = a > umax - b;
  }
  //C in subtraction is set if there is no borrow
  else {
    sRegisters.pstate.C = a >= b;
  }
}