#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "utility.h"

//Writes unless zero reg, since 32 bits, moves to the front of register by LSL
void write32(Register* reg, uint32_t val) {
  if (reg == zeroReg) return;
  *reg = val & 0xFFFFFFFF;
}

//Writes unless zero reg
void write64(Register* reg, uint64_t val) {
  if (reg == zeroReg) return;
  *reg = val;
}

//Reads, since only first 32 bits LSR
uint32_t read32(Register* reg) {
  return *reg & 0xFFFFFFFF;
}

//Reads
uint64_t read64(Register* reg) {
  return *reg;
}

uint32_t fetch32(int index) {
  return memory[index] + (memory[index+1] << 8) + (memory[index+2] << 16) + (memory[index+3] << 24);
}

uint32_t twos(uint32_t num) {
  return (~num) + 1;
}

// Returns the instruction masked from bits a to b inclusive, ensure a >= b, returns 0 with error
uint32_t mask32_AtoB_shifted(uint32_t instruction, uint8_t a, uint8_t b) {
  // Check for invalid bit positions
  if (a < b || a > 31 || b > 31) {
    fprintf(stderr, "Error: Invalid a: %u or b: %u\n", a, b);
  }
  // Create mask from a and b via 1 shifted by difference of a and b + 1 minus 1 then shifted by a
  uint32_t mask = ((1U << (a - b + 1)) - 1) << b;
  // Return instruction masked
  return (instruction & mask) >> b;
}

uint64_t mask64_AtoB_shifted(uint64_t instruction, uint8_t a, uint8_t b) {
    if (a < b || a > 63 || b > 63) {
    fprintf(stderr, "Error: Invalid a: %u or b: %u\n", a, b);
  }

  uint64_t mask = ((1U << (a - b + 1)) - 1) << b;
  return (instruction & mask) >> b;
}

void setFlagsLogical(uint64_t result, uint64_t a, uint64_t b, bool b64) {
  // Find sign bits of a, b, result
  bool rSign;
  if (b64) {
    rSign = signBit(result, 63);
  } else {
    rSign = signBit(result, 31);
  }
  
  //N is set to the sign bit of the result
  sRegisters.pstate.N = rSign;

  //Z is set only when the result is all zero
  sRegisters.pstate.Z = !result;

  // C & V set to 0
  sRegisters.pstate.C = 0;
  sRegisters.pstate.V = 0;
}

void setFlagsArithmetic(uint64_t result, uint64_t a, uint64_t b, bool addition, bool bit64) {
  setFlagsLogical(result, a, b, bit64);
  
  // Find sign bits of a, b, result
  bool aSign;
  bool bSign;
  bool rSign;
  if (bit64) {
    aSign = signBit(a, 63);
    bSign = signBit(b, 63);
    rSign = signBit(result, 63);
  } else {
    aSign = signBit(a, 31);
    bSign = signBit(b, 31);
    rSign = signBit(result, 31);
  }

  //V is set when there is signed overflow/underflow (! is pos, else neg)
  if (addition) {
    if ((!aSign && !bSign && rSign) || (aSign && bSign && !rSign)) { 
      sRegisters.pstate.V = 1; 
    } else {
      sRegisters.pstate.V = 0; 
    }
  } else { // Subtraction
    if ((!aSign && bSign && rSign) || (aSign && !bSign && !rSign)) { 
      sRegisters.pstate.V = 1; 
    } else {
      sRegisters.pstate.V = 0; 
    }
  }

  
  //C in addition is set when an unsigned carry produced
  if (addition) {
    sRegisters.pstate.C = result < a || result < b || (result > UINT32_MAX && !bit64);
  }
  //C in subtraction is set if there is no borrow
  else {
    sRegisters.pstate.C = a >= b;
  }
}

uint64_t sExtend64(uint64_t value, int msb) {
  if (value & (1 << msb)) { //If msb
    uint64_t bits = 0;
    bits = bits - 0b1;
    bits = (bits >> (msb + 1)) << (msb + 1);
    return value | bits;
  }
  return value;
}

uint32_t sExtend32(uint32_t value, int msb) {
  if (value & (1 << msb)) { //If msb
    uint64_t bits = 0;
    bits = bits - 0b1;
    bits = (bits >> (msb + 1)) << (msb + 1);
    return value | bits;
  }
  return value;
}

void logicalShiftLeft(uint64_t *value, uint8_t shift) {
  *value = ((uint64_t)*value << shift);
}

void logicalShiftRight(uint64_t *value, uint8_t shift) {
  *value = *value >> shift;
}

void arithmeticShiftRight(uint64_t *value, uint8_t shift, bool sf) {
  uint64_t signBit;
  if (sf) { // 64
    signBit = (*value >> 63) << 63;
  } else { // 32
    signBit = (*value >> 31) << 31;
  }
  if (signBit) {
    //Put it to the right position based on sf
    for (int i = 0; i < shift; i++) {
      *value = (*value >> 1) | signBit;
    }
  } else { //Bit is 0 so will be the same effect as lsr
    *value = *value >> shift;
  }
}

void rotateRight(uint64_t *value, uint8_t shift, bool sf) {
  uint16_t Rshift;
  if (sf) {
    Rshift = 63;
  } else {
    Rshift = 31;
  }

  for (int i = 0; i < shift; i++) {
    //Shifts op2 to right and wraps round last bit
    *value = (*value >> 1) | ((*value & 0b1) << Rshift);
  }
}

bool signBit(uint64_t value, uint8_t msb) {
  return (value << (63 - msb)) >> 63;
}
