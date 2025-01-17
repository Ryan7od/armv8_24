#include "emulate.h"

#define zeroReg &sRegisters.Zero

#define ERROR(msg) \
    fprintf(stderr, "Error in file: %s, line: %d\n", __FILE__, __LINE__); \
    fprintf(stderr, "Message: %s", msg); \
    exit(-1);

#define ERROR_A(msg, ...) \
    fprintf(stderr, "Error in file: %s, line: %d\n", __FILE__, __LINE__); \
    fprintf(stderr, "Message: " msg "\n", __VA_ARGS__); \
    exit(-1);

extern uint32_t twos(uint32_t num);
extern uint32_t mask32_AtoB_shifted(uint32_t instruction, uint8_t a, uint8_t b);
extern uint64_t mask64_AtoB_shifted(uint64_t instruction, uint8_t a, uint8_t b);
extern void setFlagsLogical(uint64_t result, uint64_t a, uint64_t b, bool b64);
extern void setFlagsArithmetic(uint64_t result, uint64_t a, uint64_t b, bool addition, bool bit64);
extern uint64_t sExtend64(uint64_t value, int msb);
extern uint32_t sExtend32(uint32_t value, int msb);
extern void logicalShiftLeft(uint64_t *value, uint8_t shift);
extern void logicalShiftRight(uint64_t *value, uint8_t shift);
extern void arithmeticShiftRight(uint64_t *value, uint8_t shift, bool sf);
extern void rotateRight(uint64_t *value, uint8_t shift, bool sf);
extern bool signBit(uint64_t value, uint8_t msb);
extern void write32(Register* reg, uint32_t val);
extern uint32_t read32(Register* reg);
extern void write64(Register* reg, uint64_t val);
extern uint64_t read64(Register* reg);
extern uint32_t fetch32(int index);