//
// Created by maxdstoddard on 6/9/24.
//

#include <stdlib.h>
#include "memReg.h"



//Global variables
//2MB of memory
unsigned char memory[MB2] = { 0 };
Register gRegisters[31] = { 0 };
SpecialRegisters sRegisters = { 0, 0, { false, true, false, false } };