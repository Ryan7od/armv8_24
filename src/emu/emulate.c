#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#ifndef emulate
#define emulate


#include "memReg.h"
#include "utility.h"
#include "loadstore.h"

#endif

//Prototype functions
extern void printEnd(FILE *ptr);
extern void dataProcessingImmHandler(uint32_t instruction);
extern void dataProcessingImmArithHandler(uint32_t instruction);
extern void dataProcessingImmWideMoveHandler(uint32_t instruction);
extern void dataProcessingRegHandler(uint32_t instruction);
extern void dataProcessingRegArithHandler(uint32_t instruction);
extern void dataProcessingRegLogicHandler(uint32_t instruction);
extern void dataProcessingRegMultHandler(uint32_t instruction);
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

void dataProcessingRegHandler(uint32_t instruction) {
  const uint8_t m = mask32_AtoB_shifted(instruction, 28, 28);
  const uint8_t opr = mask32_AtoB_shifted(instruction, 24, 21);

  if (m) { // M = 1
    if (opr == 0b1000) //Just to confirm correct instruction even though mult is the only case of M=1
      dataProcessingRegMultHandler(instruction);
    else {
      fprintf(stderr, "Unknown instruction with m flag = 1\n");
    }
  } else { // M = 0
    switch (opr) {
      //Arithmetic (1xx0)
      case (0b1110):
      case (0b1100):
      case (0b1010):
      case (0b1000):
        dataProcessingRegArithHandler(instruction);
        break;
      //Bit-Logic (0xxx)
      case (0b0000):
      case (0b0001):
      case (0b0010):
      case (0b0011):
      case (0b0100):
      case (0b0101):
      case (0b0110):
      case (0b0111):
        dataProcessingRegLogicHandler(instruction);
        break;
      default:
        fprintf(stderr, "Unknown opr with M=0 in dpRegisterHandle (%x)", opr);
        break;
    }
  }
}

void dataProcessingRegArithHandler(uint32_t instruction) {
  const uint8_t shift = mask32_AtoB_shifted(instruction, 23, 22);
  const uint8_t rd = mask32_AtoB_shifted(instruction, 4, 0);
  const uint8_t rn = mask32_AtoB_shifted(instruction, 9, 5);
  const uint8_t operand = mask32_AtoB_shifted(instruction, 15, 10);
  const uint8_t rm = mask32_AtoB_shifted(instruction, 20, 16);
  const uint8_t opc = mask32_AtoB_shifted(instruction, 30, 29);
  const bool sf = mask32_AtoB_shifted(instruction, 31, 31);

  uint64_t op1;
  uint64_t op2;
  if (sf) {
    op1 = read64(&gRegisters[rn]);
    op2 = read64(&gRegisters[rm]);
  } else {
    op1 = read32(&gRegisters[rn]);
    op2 = read32(&gRegisters[rm]);
  }

  //Shift op2
  switch (shift) {
    //lsl
    case (0b00):
      logicalShiftLeft(&op2, operand);
      break;
    //lsr
    case (0b01):
      logicalShiftRight(&op2, operand);
      break;
    //asr
    case (0b10):
      arithmeticShiftRight(&op2, operand, sf);
      break;
    default:
      fprintf(stderr, "Error: Shift is unknown (%x)", shift);
      break;
  }

  //Make sure value is still 32 bits
  if (!sf) {
    op2 = (uint32_t) op2;
  }

  uint64_t result;
  if (opc & 0b10) { //Sub
    result = op1 - op2;
  } else { //Add
    result = op1 + op2;
  }

  if (opc & 0b01) { //Set flags
    setFlagsArithmetic(result, op1, op2, !(opc & 0b10), sf);
  }


  //TODO: As far as I'm aware we dont have to code for 1111 since it "also" encodes ZR and that does nothing, but it still encodes R31
  //Set value based on sf
  if (sf) {
    write64(&gRegisters[rd], result);
  } else {
    write32(&gRegisters[rd], result);
  }
}

void dataProcessingRegLogicHandler(uint32_t instruction) {
  const uint8_t rd = mask32_AtoB_shifted(instruction, 4, 0);
  const uint8_t rn = mask32_AtoB_shifted(instruction, 9, 5);
  const uint8_t operand = mask32_AtoB_shifted(instruction, 15, 10);
  const uint8_t rm = mask32_AtoB_shifted(instruction, 20, 16);
  const uint8_t n = mask32_AtoB_shifted(instruction, 21, 21);
  const uint8_t shift = mask32_AtoB_shifted(instruction, 23, 22);
  const uint8_t opc = mask32_AtoB_shifted(instruction, 30, 29);
  const bool sf = mask32_AtoB_shifted(instruction, 31, 31);


  uint64_t op1;
  uint64_t op2;
  if (sf) {
    op1 = read64(&gRegisters[rn]);
    op2 = read64(&gRegisters[rm]);
  } else {
    op1 = read32(&gRegisters[rn]);
    op2 = read32(&gRegisters[rm]);
  }

    // Ensure 64 or 32 after shift

  //Shift op2
  switch (shift) {
    //lsl
    case (0b00):
      logicalShiftLeft(&op2, operand);
      break;
    //lsr
    case (0b01):
      logicalShiftRight(&op2, operand);
      break;
    //asr
    case (0b10):
      arithmeticShiftRight(&op2, operand, sf);
      break;
    case (0b11): //ror
      rotateRight(&op2, operand, sf);
      break;
    default:
      fprintf(stderr, "Error: shift unknown in dpRegLogicHandler (%x)", shift);
      break;
  }

  //Make sure 32 bits if !sf
  if (!sf) {
    op2 = (uint32_t)op2;
  }

  //Negate op2 based on N flah
  if (n) {
    op2 = ~op2;
  }

  uint64_t result;
  switch (opc) {
    // and/bic
    case (0b00):
      result = op1 & op2;
      break;
    // orr/orn
    case (0b01):
      result = op1 | op2;
      break;
    // eor/eon
    case (0b10):
      result = op1 ^ op2;
      break;
    // ands/bics
    case (0b11):
      result = op1 & op2;
      setFlagsLogical(result, op1, op2, sf);
      break;
    default:
      fprintf(stderr, "Unknown opc in dpRegisterLogicHandler (%x)", opc);
      break;
  }


  //TODO: As far as I'm aware we dont have to code for 1111 since it "also" encodes ZR and that does nothing, but it still encodes R31
  //Set value based on sf
  if (sf) {
    write64(&gRegisters[rd], result);
  } else {
    write32(&gRegisters[rd], result);
  }
}

void dataProcessingRegMultHandler(uint32_t instruction) {
  const uint8_t rd = mask32_AtoB_shifted(instruction, 4,  0);
  const uint8_t rn = mask32_AtoB_shifted(instruction, 9,  5);
  const uint8_t ra = mask32_AtoB_shifted(instruction, 14, 10);
  const uint8_t x  = mask32_AtoB_shifted(instruction, 15, 15);
  const uint8_t rm = mask32_AtoB_shifted(instruction, 20, 16);
  const uint8_t sf = mask32_AtoB_shifted(instruction, 31, 31);

  if (rd == 0b11111) {
    return;
  }

  // Initialise op_a, op_n, op_m to 0, also handles case of zero register
  uint64_t op_a = 0; // ra
  uint64_t op_n = 0; // rn
  uint64_t op_m = 0;
  if (sf) {
    if (ra != 0b11111) op_a = read64(&gRegisters[ra]);
    if (rn != 0b11111) op_n = read64(&gRegisters[rn]);
    if (rm != 0b11111) op_m = read64(&gRegisters[rm]);
  } else {
    if (ra != 0b11111) op_a = read32(&gRegisters[ra]);
    if (rn != 0b11111) op_n = read32(&gRegisters[rn]);
    if (rm != 0b11111) op_m = read32(&gRegisters[rm]);
  }

  uint64_t product = op_n * op_m;

  uint64_t result = 0;
  if (x) { //Sub
    result = op_a - product;
  } else { //Add
    result = op_a + product;
  }

  //Set value based on sf
  if (sf) {
    write64(&gRegisters[rd], result);
  } else {
    write32(&gRegisters[rd], result);
  }
}

void dataProcessingImmHandler(uint32_t instruction) {
  const uint8_t opi = mask32_AtoB_shifted(instruction, 25, 23);

  switch (opi) {
    case 0b010:
      dataProcessingImmArithHandler(instruction);
      break;
    case 0b101:
      dataProcessingImmWideMoveHandler(instruction);
      break;
    default:
      fprintf(stderr, "Error: Unknown data processing immediate instruction");
      break;
  }
}

void dataProcessingImmWideMoveHandler(uint32_t instruction) {
  const uint8_t rd = mask32_AtoB_shifted(instruction, 4, 0);
  const uint8_t hw = mask32_AtoB_shifted(instruction, 22, 21);
  const uint8_t opc = mask32_AtoB_shifted(instruction, 30, 29);
  const uint8_t sf = mask32_AtoB_shifted(instruction, 31, 31);
  uint64_t imm16 = mask32_AtoB_shifted(instruction, 20, 5);

  imm16 = imm16 << (hw << 4); //Shifted by hw * 16

  uint64_t mask = 0; // Initialise mask
  switch (opc) {
    //movn
    case (0b00):
      if (sf) {
        write64(&gRegisters[rd], ~imm16);
      } else {
        write32(&gRegisters[rd], ~imm16);
      }
      break;
    //movz
    case (0b10):
      if (sf) {
        write64(&gRegisters[rd], imm16);
      } else {
        write32(&gRegisters[rd], imm16);
      }
      break;
    //movk
    case (0b11):
      mask = 0xffff;
      mask = mask << (hw << 4);
      //Mask used to keep values from last register
      if (sf) {
        uint64_t write = (read64(&gRegisters[rd]) & ~mask) | imm16;
        write64(&gRegisters[rd], write);
      } else {
        uint32_t write = (read32(&gRegisters[rd]) & ~mask) | imm16;
        write32(&gRegisters[rd], write);
      }
      break;
    default:
      fprintf(stderr, "Error: Unknown opc in ImmWidMov (%x)", opc);
      break;

  }
}

void dataProcessingImmArithHandler(uint32_t instruction) {
  const uint8_t rd = mask32_AtoB_shifted(instruction, 4, 0);
  const uint8_t opc = mask32_AtoB_shifted(instruction, 30, 29);
  const uint8_t sf = mask32_AtoB_shifted(instruction, 31, 31);
  const uint8_t sh = mask32_AtoB_shifted(instruction, 22, 22);
  const uint8_t rn = mask32_AtoB_shifted(instruction, 9, 5);
  uint32_t imm12 = mask32_AtoB_shifted(instruction, 21, 10);

  //Do shift if necessary
  if (sh) {
    imm12 = imm12 << 12;
  }

  //Fetch value in Rn
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
    setFlagsArithmetic(result, operand, imm12, !(opc & 0b10), sf);
  }


  //TODO: As far as I'm aware we dont have to code for 1111 since it "also" encodes ZR and that does nothing, but it still encodes R31
  //Set value based on sf
  if (sf) {
    write64(&gRegisters[rd], result);
  } else {
    write32(&gRegisters[rd], result);
  }
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