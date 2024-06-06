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
extern void dataProcessingImmWideMoveHandler(uint32_t instruction);
extern void dataProcessingRegHandler(uint32_t instruction);
extern void dataProcessingRegArithHandler(uint32_t instruction);
extern void dataProcessingRegLogicHandler(uint32_t instruction);
extern void dataProcessingRegMultHandler(uint32_t instruction);
extern void loadStoreHandler(uint32_t instruction);
extern void loadStoreHelperHandler(uint8_t l, uint8_t sf, uint8_t rt, uint64_t address);
extern void loadStoreLoadLiteralHandler(uint32_t instruction);
extern void loadStoreUnsignedOffsetHandler(uint32_t instruction);
extern void loadStoreRegOffHandler(uint32_t instruction);
extern void loadStorePIndexHandler(uint32_t instruction);
extern void branchHandler(uint32_t instruction);
extern uint32_t twos(uint32_t num);
extern uint32_t mask32_AtoB_shifted(uint32_t instruction, uint8_t a, uint8_t b);
extern void setFlags(uint64_t result, uint64_t a, uint64_t b, bool addition, bool bit64);
extern uint64_t sExtend64(uint64_t value, int msb);
extern uint32_t sExtend32(uint32_t value, int msb);

//Global variables
//2MB of memory
unsigned char memory[MB2] = { 0 };
Register gRegisters[31] = { 0 };
SpecialRegisters sRegisters = { 0, 0, { false, true, false, false } };

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
  const uint8_t sf = mask32_AtoB_shifted(instruction, 31, 31);

  uint64_t op1;
  if (sf) {
    op1 = read64(&gRegisters[rn]);
  } else {
    op1 = read32(&gRegisters[rn]);
  }

  uint64_t op2;
  if (sf) {
    op2 = read64(&gRegisters[rm]);
  } else {
    op2 = read32(&gRegisters[rm]);
  }

  uint64_t newBit = 0; // Initialise newBit
  //Shift op2
  switch (shift) {
    //lsl
    case (0b00):
      op2 = op2 << operand;
      break;
    //lsr
    case (0b01):
      op2 = op2 >> operand;
      break;
    //asr
    case (0b10):

      newBit = (operand & 0b1000) >> 3;
      if (newBit) {
        //Put it to the right position based on sf
        if (sf) newBit = newBit << 31; else newBit = newBit << 63;
        for (int i = 0; i < operand; i++) {
          op2 = (op2 >> 1) | newBit;
        }
      } else { //Bit is 0 so will be the same effect as lsr
        op2 = op2 >> operand;
      }
      break;
    default:
      break;
  }

  //Make sure value is still 32 bits if sf
  if (sf) {
    op2 = (uint32_t) op2;
  }

  uint64_t result;
  if (opc & 0b10) { //Sub
    result = op1 - op2;
  } else { //Add
    result = op1 + op2;
  }

  if (opc & 0b01) { //Set flags
    setFlags(result, op1, op2, !(opc & 0b10), sf);
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
  const uint8_t sf = mask32_AtoB_shifted(instruction, 31, 31);


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

  int Rshift;
  uint64_t op2signBit;

  //Shift op2
  switch (shift) {
    //lsl
    case (0b00):
      op2 = op2 << operand;
      break;
    //lsr
    case (0b01):
      op2 = op2 >> operand;
      break;
    //asr
    case (0b10):
      if (sf) { // 64
        op2signBit = (op2 >> 63) << 63;
      } else { // 32
        op2signBit = (op2 >> 31) << 31;
      }
      if (op2signBit) {
        //Put it to the right position based on sf
        for (int i = 0; i < operand; i++) {
          op2 = (op2 >> 1) | op2signBit;
        }
      } else { //Bit is 0 so will be the same effect as lsr
        op2 = op2 >> operand;
      }
      break;
    //ror
    case (0b11):
      //Rotation based on bits
      if (sf) Rshift = 63; else Rshift = 31;
      for (int i = 0; i < operand; i++) {
        //Shifts op2 to right and wraps round last bit
        op2 = (op2 >> 1) | ((op2 & 0b1) << Rshift);
      }
      break;
    default:
      break;
  }

  //Make sure 32 bits if !sf
  if (!sf) {
    op2 = (uint32_t) op2;
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
      //Set flags
      //C and V 0
      sRegisters.pstate.C = false;
      sRegisters.pstate.V = false;
      //Z if result is zero
      sRegisters.pstate.Z = !result;
      //N as sign of result (depends on 32 or 64 bit)
      if (sf) {
        sRegisters.pstate.N = result & 0x80000000;
      } else {
        sRegisters.pstate.N = result & 0x8000000000000000;
      }
      break;
    default:
      result = 0;
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
      fprintf(stderr, "Error: Unknown instruction (ImmWidMov)");
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
    setFlags(result, operand, imm12, !(opc & 0b10), sf);
  }


  //TODO: As far as I'm aware we dont have to code for 1111 since it "also" encodes ZR and that does nothing, but it still encodes R31
  //Set value based on sf
  if (sf) {
    write64(&gRegisters[rd], result);
  } else {
    write32(&gRegisters[rd], result);
  }
}

void loadStoreHandler(uint32_t instruction) {
  const uint8_t diff = mask32_AtoB_shifted(instruction, 31, 31);

  if (!diff) { //Load literal
    loadStoreLoadLiteralHandler(instruction);
  } else {
    const uint8_t u = mask32_AtoB_shifted(instruction, 24, 24);
    if (u) { //Unsigned offset
      loadStoreUnsignedOffsetHandler(instruction);
    } else {
      const uint8_t diff2 = mask32_AtoB_shifted(instruction, 21, 21);
      if (diff2) { //Register offset
        loadStoreRegOffHandler(instruction);
      } else { // Pre/Post-index
        loadStorePIndexHandler(instruction);
      }
    }
  }
}

void loadStoreLoadLiteralHandler(uint32_t instruction) {
  const uint32_t simm19 = mask32_AtoB_shifted(instruction, 23, 5);
  const uint8_t sf = mask32_AtoB_shifted(instruction, 30, 30);
  const uint8_t rt = mask32_AtoB_shifted(instruction, 4, 0);

  uint64_t offset = simm19 << 2;
  offset = sExtend64(offset, 20);

  uint64_t address = read64(&sRegisters.PC) + offset;

  if (sf) { //64 bit
    uint64_t result = 0;
    for (int i = 0; i < 8; i++) {
      uint64_t mem_i = memory[address + i];
      uint64_t mem_i_shifted = mem_i << (8 * i);
      result = result + mem_i_shifted;
    }
    write64(&gRegisters[rt], result);
  } else { //32 bit
    uint32_t result = 0;
    for (int i = 0; i < 4; i++) {
      result += memory[address + i] << (8 * i);
    }
    write32(&gRegisters[rt], result);
  }
}

void loadStoreHelperHandler(uint8_t l, uint8_t sf, uint8_t rt, uint64_t address) {
  if (l) { //Load
    if (sf) {
      uint64_t data = 0;
      for (int i = 0; i < 8; i++) {
        data += memory[address + i] << (4 * i);
      }
      write64(&gRegisters[rt], data);
    } else {
      uint32_t data = 0;
      for (int i = 0; i < 4; i++) {
        data += memory[address + i] << (4 * i);
      }
      write32(&gRegisters[rt], data);
    }
  } else { //Store
    if (sf) {
      uint64_t data = read64(&gRegisters[rt]);
      for (int i = 0; i < 8; i++) {
        memory[address + i] = data & 0xFF;
        data = data >> 8;
      }
    } else {
      uint32_t data = read32(&gRegisters[rt]);
      for (int i = 0; i < 4; i++) {
        memory[address + i] = data & 0xFF;
        data = data >> 8;
      }
    }
  }
}

void loadStoreUnsignedOffsetHandler(uint32_t instruction) {
  const uint8_t xn = mask32_AtoB_shifted(instruction, 9, 5);
  const uint8_t sf = mask32_AtoB_shifted(instruction, 30, 30);
  const uint8_t rt = mask32_AtoB_shifted(instruction, 4, 0);
  const uint16_t imm12 = mask32_AtoB_shifted(instruction, 21, 10);
  const uint8_t l = mask32_AtoB_shifted(instruction, 22, 22);

  uint64_t uoffset;
  int msb;
  if (sf) {
    uoffset = imm12 << 3;
    msb = 14;
  } else {
    uoffset = imm12 << 2;
    msb = 13;
  }

  uint64_t address = sExtend64(uoffset, msb) + read64(&gRegisters[xn]);

  loadStoreHelperHandler(l, sf, rt, address);
}

void loadStoreRegOffHandler(uint32_t instruction) {
  const uint8_t xn = mask32_AtoB_shifted(instruction, 9, 5);
  const uint8_t sf = mask32_AtoB_shifted(instruction, 30, 30);
  const uint8_t rt = mask32_AtoB_shifted(instruction, 4, 0);
  const uint16_t xm = mask32_AtoB_shifted(instruction, 20, 16);
  const uint8_t l = mask32_AtoB_shifted(instruction, 22, 22);

  uint64_t address = read64(&gRegisters[xm]) + read64(&gRegisters[xn]);

  loadStoreHelperHandler(l, sf, rt, address);
}

void loadStorePIndexHandler(uint32_t instruction) {
  const uint8_t xn = mask32_AtoB_shifted(instruction, 9, 5);
  const uint8_t sf = mask32_AtoB_shifted(instruction, 30, 30);
  const uint8_t rt = mask32_AtoB_shifted(instruction, 4, 0);
  const uint16_t simm9 = mask32_AtoB_shifted(instruction, 20, 12);
  const uint8_t l = mask32_AtoB_shifted(instruction, 22, 22);
  const uint8_t i = mask32_AtoB_shifted(instruction, 11, 11);

  if (i) { //Pre-indexed
    uint64_t address = read64(&gRegisters[xn]) + sExtend64(simm9, 8);
    write64(&gRegisters[xn], address);
    loadStoreHelperHandler(l, sf, rt, address);
  } else { //Post-indexed
    uint64_t address = read64(&gRegisters[xn]);
    loadStoreHelperHandler(l, sf, rt, address);
    address +=  + sExtend64(simm9, 8);
    write64(&gRegisters[xn], address);
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
