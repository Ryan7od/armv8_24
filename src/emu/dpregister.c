#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "utility.h"
#include "dpregister.h"


static void dataProcessingRegArithHandler(uint32_t instruction) {
    const uint8_t shift = mask32_AtoB_shifted(instruction, 23, 22);
    const uint8_t rd = mask32_AtoB_shifted(instruction, 4, 0);
    const uint8_t rn = mask32_AtoB_shifted(instruction, 9, 5);
    const uint8_t operand = mask32_AtoB_shifted(instruction, 15, 10);
    const uint8_t rm = mask32_AtoB_shifted(instruction, 20, 16);
    const uint8_t opc = mask32_AtoB_shifted(instruction, 30, 29);
    const bool sf = mask32_AtoB_shifted(instruction, 31, 31);

    uint64_t op1;
    uint64_t op2;
    if (sf) {
        op1 = read64(&gRegisters[rn]);
        op2 = read64(&gRegisters[rm]);
    } else {
        op1 = read32(&gRegisters[rn]);
        op2 = read32(&gRegisters[rm]);
    }

    //Shift op2
    switch (shift) {
        //lsl
        case (0b00):
            logicalShiftLeft(&op2, operand);
            break;
            //lsr
        case (0b01):
            logicalShiftRight(&op2, operand);
            break;
            //asr
        case (0b10):
            arithmeticShiftRight(&op2, operand, sf);
            break;
        default:
            ERROR_A("Shift unknown: %x", shift);
            break;
    }

    //Make sure value is still 32 bits
    if (!sf) {
        op2 = (uint32_t) op2;
    }

    uint64_t result;
    if (opc & 0b10) { //Sub
        result = op1 - op2;
    } else { //Add
        result = op1 + op2;
    }

    if (opc & 0b01) { //Set flags
        setFlagsArithmetic(result, op1, op2, !(opc & 0b10), sf);
    }


    //TODO: As far as I'm aware we dont have to code for 1111 since it "also" encodes ZR and that does nothing, but it still encodes R31
    //Set value based on sf
    if (sf) {
        write64(&gRegisters[rd], result);
    } else {
        write32(&gRegisters[rd], result);
    }
}

static void dataProcessingRegLogicHandler(uint32_t instruction) {
    const uint8_t rd = mask32_AtoB_shifted(instruction, 4, 0);
    const uint8_t rn = mask32_AtoB_shifted(instruction, 9, 5);
    const uint8_t operand = mask32_AtoB_shifted(instruction, 15, 10);
    const uint8_t rm = mask32_AtoB_shifted(instruction, 20, 16);
    const uint8_t n = mask32_AtoB_shifted(instruction, 21, 21);
    const uint8_t shift = mask32_AtoB_shifted(instruction, 23, 22);
    const uint8_t opc = mask32_AtoB_shifted(instruction, 30, 29);
    const bool sf = mask32_AtoB_shifted(instruction, 31, 31);


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

    //Shift op2
    switch (shift) {
        //lsl
        case (0b00):
            logicalShiftLeft(&op2, operand);
            break;
            //lsr
        case (0b01):
            logicalShiftRight(&op2, operand);
            break;
            //asr
        case (0b10):
            arithmeticShiftRight(&op2, operand, sf);
            break;
        case (0b11): //ror
            rotateRight(&op2, operand, sf);
            break;
        default:
            ERROR_A("Shift unknown: %x", shift);
            break;
    }

    //Make sure 32 bits if !sf
    if (!sf) {
        op2 = (uint32_t)op2;
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
            setFlagsLogical(result, op1, op2, sf);
            break;
        default:
            ERROR_A("Unknown opc: %x", opc);
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

static void dataProcessingRegMultHandler(uint32_t instruction) {
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
    uint64_t op_m = 0; // rm
    
    if (ra != 0b11111) op_a = sf ? read64(&gRegisters[ra]) : read32(&gRegisters[ra]);
    if (rn != 0b11111) op_n = sf ? read64(&gRegisters[rn]) : read32(&gRegisters[rn]);
    if (rm != 0b11111) op_m = sf ? read64(&gRegisters[rm]) : read32(&gRegisters[rm]);

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

void dataProcessingRegHandler(uint32_t instruction) {
    const uint8_t m = mask32_AtoB_shifted(instruction, 28, 28);
    const uint8_t opr = mask32_AtoB_shifted(instruction, 24, 21);

    if (m) { // M = 1
        if (opr == 0b1000) //Just to confirm correct instruction even though mult is the only case of M=1
            dataProcessingRegMultHandler(instruction);
        else {
            ERROR_A("Unknown opr with M=1: %x", opr)
        }
    } else { // M = 0
        if (opr & 0b1001 == 0b1000) { //Arithmetic (1xx0)
            dataProcessingRegArithHandler(instruction);
        } else if (opr & 0b1000 == 0b0000) { //Bit-Logic (0xxx)
            dataProcessingRegLogicHandler(instruction);
        } else { // Unknown
            ERROR_A("Unknown opr with M=0: %x", opr);
        }
    }
}