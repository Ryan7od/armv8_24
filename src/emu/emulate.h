#include <stdio.h>
#include <stdarg.h>
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
extern unsigned char memory[MB2];
extern Register gRegisters[31];
extern SpecialRegisters sRegisters;