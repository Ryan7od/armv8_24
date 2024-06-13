#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "utility.h"
#include "branches.h"

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

