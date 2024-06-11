#include <stdlib.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

extern void dataProcessingRegHandler(uint32_t instruction);
extern void dataProcessingRegArithHandler(uint32_t instruction);
extern void dataProcessingRegLogicHandler(uint32_t instruction);
extern void dataProcessingRegMultHandler(uint32_t instruction);
