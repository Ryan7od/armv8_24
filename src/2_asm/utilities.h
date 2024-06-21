#include "assemble.h"

extern void writeToFile(uint32_t write_val, FILE *file);
extern uint32_t getN(const char *opcode);
extern uint32_t getimmm(char *num);
extern uint32_t getSf(char *reg);
extern uint32_t getOpcode(InstructionIR instructionIr, OpcodeMapping mapping[], size_t size);
extern uint32_t getN(const char *opcode);
extern uint32_t getOpr(char *operand);
extern char** allocateRegisters(InstructionIR instruction);
extern uint32_t getEncoding(const char* mnemonic);
extern char * trim_leading_spaces(char *str);
extern int getAddress(dynarray symbolTable, const char *label);
uint32_t getReg(char *reg);