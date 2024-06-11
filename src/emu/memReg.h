#ifndef MemReg
#define MemReg

#endif MemReg

#define MB2 2097152 // 2MB of memory (2*2^20)

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

//Global variables
//2MB of memory
unsigned char memory[MB2] = { 0 };
Register gRegisters[31] = { 0 };
SpecialRegisters sRegisters = { 0, 0, { false, true, false, false } };