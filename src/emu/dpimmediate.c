#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "utility.h"
#include "dpimmediate.h"


static void dataProcessingImmWideMoveHandler(uint32_t instruction) {
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

static void dataProcessingImmArithHandler(uint32_t instruction) {
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
